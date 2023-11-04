// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "GameStateApplier.h"
#include "PlayerPawn.h"


#include "Animation/AnimInstance.h" /////////////// TESTING DEBUG TODO FIXME

GameStateApplier::GameStateApplier()
{
}

GameStateApplier::~GameStateApplier()
{
}

bool GameStateApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
	if (!PacketApplier::Init(session, gameInstance)) return false;
	return true;
}

bool GameStateApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
	URovenhellGameInstance* gameInstance = Cast<URovenhellGameInstance>(GameInstance);
	if (!gameInstance) return false;

	bool applied = true;
	switch (gameInstance->GetExecType()->GetHostType())
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
			{
				applied &= ApplyPacket_UEClient(packet, netHandler);
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				applied &= false;
				UE_LOG(LogTemp, Error, TEXT("로직서버는 GameState 패킷을 처리하지 않아야 합니다."));
				break;
			}
	}
    return applied;
}

bool GameStateApplier::ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
	// 서버 틱과 동기화
	netHandler->GetRovenhellGameInstance()->TickCounter->SetServerTick_UEClient(packet->GetHeader()->senderTick);

	// 패킷 순서 검증 (TCP지만 만일의 사태를 대비)
	if (!netHandler->GetDeserializerShared()->IsCorrectPacket(packet->GetHeader()))
	{
		netHandler->GetDeserializerShared()->ResetPacketInfo();
		netHandler->GetDeserializerShared()->Clear(); // 그동안 수신한 버퍼 Fragment 삭제
		return true;
	}

	// 버퍼 복사
	netHandler->GetDeserializerShared()->SetPacketInfo(packet->GetHeader());
	netHandler->GetDeserializerShared()->ReadDataFromBuffer(packet);

	// 마지막 fragment일 경우
	if (packet->GetHeader()->packetOrder == packet->GetHeader()->fragmentCount)
	{
		SD_GameState* gameState = new SD_GameState();
		netHandler->GetDeserializerShared()->Deserialize((SD_Data*)gameState);
		ApplyGameState_UEClient(gameState, netHandler); // 처리
		netHandler->GetDeserializerShared()->ResetPacketInfo();
		netHandler->GetDeserializerShared()->Clear(); // 처리를 완료한 버퍼 Fragment 삭제
	}
	return true;
}

void GameStateApplier::ApplyGameState_UEClient(SD_GameState* gameState, ANetHandler* netHandler)
{
	for (auto& playerConnection : IsPlayerConnected)
	{
		// NONEXISTENT인 세션은 Disconnect 해선 안됨 (nullptr dereference 에러남)
		if (playerConnection.Value == ConnectionStatus::CONNECTED)
		{
			playerConnection.Value = ConnectionStatus::DISCONNECTED;
		}
	}

	// 접속해있는 플레이어들(호스트 플레이어 포함)에 대한 처리
	for (SD_PlayerState playerState : gameState->UpdatedPlayerStates)
	{
		CheckForNewConnection_UEClient(playerState.SessionId, netHandler); // 새로 접속했을 경우 알맞는 처리를 해주고 폰 생성
		IsPlayerConnected.Add(playerState.SessionId, ConnectionStatus::CONNECTED); // 정보 업데이트 혹은 추가
		ApplyPlayerPhysics_UEClient(gameState, playerState.SessionId, playerState.PlayerPhysics, netHandler); // 나머지 로직 처리
		ApplyPlayerAnimation_UEClient(&playerState, netHandler);
	}

	// 접속 종료된 세션 처리
	for (auto& playerConnection : IsPlayerConnected)
	{
		if (playerConnection.Value == ConnectionStatus::DISCONNECTED)
		{
			OnNewClientDisconnection_UEClient(playerConnection.Key, netHandler);
			playerConnection.Value = ConnectionStatus::NONEXISTENT;
		}
	}
}

void GameStateApplier::ApplyPlayerPhysics_UEClient(SD_GameState* gameState, const uint16 playerSessionId, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler)
{
	APlayerPawn* player = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(playerSessionId);
	if (!player)
	{
		UE_LOG(LogTemp, Fatal, TEXT("$%i번 세션의 플레이어 폰이 설정되지 않았습니다!"), playerSessionId);
		return;
	}

	if (player->IsPuppet())
	{
		return ApplyPlayerPhysicsOfPuppet_UEClient(player, gameState, playerPhysics, netHandler);
	}
	else
	{
		return ApplyPlayerPhysicsOfHost_UEClient(player, gameState, playerPhysics, netHandler);
	}
}

void GameStateApplier::ApplyPlayerPhysicsOfHost_UEClient(APlayerPawn* hostPlayer, SD_GameState* gameState, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler)
{
	// 물리 보정 필요 여부 판정
	FTransform transform = playerPhysics.GetTransformFromData();
	if (!hostPlayer->GetPhysicsSyncComp()->IsActorInSyncWith(transform))
	{
		// 물리 보정
		hostPlayer->GetPhysicsSyncComp()->AdjustActorPhysics(transform);

		// 인풋 재처리
		UActorInputSyncComponent* pawnSyncComp = hostPlayer->GetInputSyncComp();
		if (pawnSyncComp)
		{
			pawnSyncComp->ReapplyLocalInputAfter(gameState->GameStateTick);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("호스트 플레이어의 인풋 싱크 컴포넌트를 찾을 수 없습니다."));
		}
	}
}

void GameStateApplier::ApplyPlayerPhysicsOfPuppet_UEClient(APlayerPawn* puppetPlayer, SD_GameState* gameState, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler)
{
	// 최신 트랜스폼 정보 추가
	FTransform transform = playerPhysics.GetTransformFromData();
	UNetPawnInterpComponent* interpComp = puppetPlayer->GetInterpComp();
	if (interpComp)
	{
		interpComp->AddNewTransform(transform, netHandler->GetRovenhellGameInstance()->TickCounter->GetTick());
		// 추가된 정보를 기반으로 움직임을 interpolate하는 과정은 해당 컴포넌트 틱에서 처리됨
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("이 퍼펫 플레이어의 인풋 싱크 컴포넌트를 찾을 수 없습니다."));
	}
}

void GameStateApplier::ApplyPlayerAnimation_UEClient(SD_PlayerState* playerState, ANetHandler* netHandler)
{
	APlayerPawn* player = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(playerState->SessionId);
	if (!player)
	{
		UE_LOG(LogTemp, Fatal, TEXT("$%i번 세션의 플레이어 폰이 설정되지 않았습니다!"), playerState->SessionId);
		return;
	}

	if (player->IsPuppet())
	{
		return ApplyPlayerAnimationOfPuppet_UEClient(player, playerState, netHandler);
	}
	else
	{
		return ApplyPlayerAnimationOfHost_UEClient(player, playerState, netHandler);
	}
}

void GameStateApplier::ApplyPlayerAnimationOfHost_UEClient(APlayerPawn* hostPlayer, SD_PlayerState* playerState, ANetHandler* netHandler)
{
	return; // 호스트 플레이어 애니메이션의 경우 호스트(클라이언트) 처리 결과만 사용함
}

void GameStateApplier::ApplyPlayerAnimationOfPuppet_UEClient(APlayerPawn* puppetPlayer, SD_PlayerState* playerState, ANetHandler* netHandler)
{
	puppetPlayer->SetAnimTo(AnimStateEnum(playerState->AnimState), playerState->AnimStatus1D);
	return;
}

void GameStateApplier::OnNewClientConnection_UEClient(uint16 clientSessionId, ANetHandler* netHandler)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("다른 플레이어의 접속이 감지되었습니다: %i번 클라이언트 세션 연결"), clientSessionId));

	// 플레이어 폰 추가
	TWeakObjectPtr<ANetActorSpawner> netSpawner = netHandler->GetRovenhellGameInstance()->GetNetActorSpawner();
	if (netSpawner != nullptr && netSpawner.IsValid())
	{
		APlayerPawn* player = netSpawner->SpawnNewPlayerPawn();
		netHandler->GetRovenhellGameInstance()->AddPlayer(clientSessionId, player);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("NetSpawner를 찾지 못했습니다."));
	return;
}

void GameStateApplier::OnNewClientDisconnection_UEClient(uint16 clientSessionId, ANetHandler* netHandler)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i번 클라이언트 세션 연결 해제"), clientSessionId));

	// 플레이어 폰 삭제
	TWeakObjectPtr<ANetActorSpawner> netSpawner = netHandler->GetRovenhellGameInstance()->GetNetActorSpawner();
	if (netSpawner != nullptr && netSpawner.IsValid())
	{
		APlayerPawn* player = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(clientSessionId);
		netHandler->GetRovenhellGameInstance()->RemovePlayer(clientSessionId);
		netSpawner->Remove(player);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("NetSpawner를 찾지 못했습니다."));
	return;
}

void GameStateApplier::CheckForNewConnection_UEClient(uint16 sessionId, ANetHandler* netHandler)
{
	// 세션 id 기반으로 플레이어 폰 접근 시도
	APlayerPawn* player = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(sessionId);

	// 새로운 세션의 접속 확인
	if (!player)
	{
		OnNewClientConnection_UEClient(sessionId, netHandler);
	}
	return;
}
