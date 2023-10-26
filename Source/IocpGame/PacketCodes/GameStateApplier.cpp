// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "GameStateApplier.h"
#include "PlayerPawn.h"

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
		// NOTE: 순서가 꼬이는 경우는 
		// 1) 멀티스레드 발송과정에서 순서가 섞였을 경우 (이 경우가 빈번할 경우 순서 맞춰주도록 로직 수정이 필요함, 그러나 아직 이런 현상이 관측되지 않으므로 보류)
		// 2) TCP 발송에서의 불량 패킷
		// 3) UEServer의 Congestion control로 인해 일부 패킷이 잘려서 발송되었을 경우
		// 정도가 있을 수 있으며, 이중 3번이 가장 가능성이 높은 경우로, 이때는 그냥 패킷을 무시해주면 된다
		// 2번은 GameState 패킷의 구조 특성상 현재로써는 모든 패킷을 반드시 받아야 하는게 아니기 때문에 그냥 패킷을 무시하고
		// 1번의 경우에는 발생한 사례가 없지만 발생한다면 순서를 보정해주는 로직의 작성이 필요하다. TODO
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

	// 접속해있는 플레이어들에 대한 처리
	for (SD_PawnPhysics playerPhysics : gameState->UpdatedPlayerPhysics)
	{
		CheckForNewConnection_UEClient(playerPhysics.SessionId, netHandler); // 새로 접속했을 경우 알맞는 처리를 해주고 폰 생성
		IsPlayerConnected.Add(playerPhysics.SessionId, ConnectionStatus::CONNECTED); // 정보 업데이트 혹은 추가
		ApplyPlayerPhysics_UEClient(gameState, playerPhysics, netHandler); // 나머지 로직 처리
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

void GameStateApplier::ApplyPlayerPhysics_UEClient(SD_GameState* gameState, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler)
{
	APlayerPawn* player = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(playerPhysics.SessionId);
	if (!player)
	{
		UE_LOG(LogTemp, Fatal, TEXT("$%i번 세션의 플레이어 폰이 설정되지 않았습니다!"), playerPhysics.SessionId);
		return;
	}

	// 물리 보정 필요 여부 판정
	FTransform transform = playerPhysics.GetTransformFromData();
	if (!player->GetPhysicsSyncComp()->IsActorInSyncWith(transform))
	{
		// 물리 보정
		player->GetPhysicsSyncComp()->AdjustActorPhysics(transform);

		// 인풋 재처리 (호스트 플레이어에 대해서만)
		class UActorInputSyncComponent* pawnSyncComp = player->GetInputSyncComp();
		if (!player->IsPuppet() && pawnSyncComp)
		{
			pawnSyncComp->ReapplyLocalInputAfter(gameState->GameStateTick);
		}
	}
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
