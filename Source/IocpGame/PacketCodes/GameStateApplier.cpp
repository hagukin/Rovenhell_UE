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
	return false; /////////////// DEBUG

	// 역직렬화
	netHandler->GetDeserializerShared()->Clear();
	SD_GameState* gameState = new SD_GameState();
	netHandler->GetDeserializerShared()->ReadDataFromBuffer(packet);
	netHandler->GetDeserializerShared()->Deserialize((SD_Data*)gameState);

	// 서버 틱과 동기화
	Cast<URovenhellGameInstance>(GameInstance)->TickCounter->SetServerTick_UEClient(gameState->Tick);

	// 데이터 처리
	ApplyPhysicsAndSyncPlayers_UEClient(gameState, netHandler);
	// TODO: GameState 내의 다른 데이터들도 처리

	return true;
}

void GameStateApplier::ApplyPhysicsAndSyncPlayers_UEClient(SD_GameState* gameState, ANetHandler* netHandler)
{
	for (SD_PawnPhysics playerPhysics : gameState->UpdatedPlayerPhysics)
	{
		// 세션 id 기반으로 플레이어 폰 접근 시도
		APlayerPawn* targetPlayer = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(playerPhysics.SessionId);

		// 새로운 세션의 접속 확인
		if (!targetPlayer)
		{
			OnNewClientConnection_UEClient(playerPhysics.SessionId, netHandler);
			targetPlayer = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(playerPhysics.SessionId);
			if (!targetPlayer)
			{
				UE_LOG(LogTemp, Fatal, TEXT("새로 접속한 클라이언트의 플레이어 폰이 정상적으로 생성되지 않았습니다."));
				continue;
			}
		}

		// 물리 보정
		FVector velocity(playerPhysics.XVelocity, playerPhysics.YVelocity, playerPhysics.ZVelocity);
		// 보정 필요 여부 판단
		if (!targetPlayer->GetPhysicsSyncComp()->IsActorInSyncWith(playerPhysics.Transform, velocity))
		{
			// 보정 처리
			targetPlayer->GetPhysicsSyncComp()->AdjustActorPhysics(gameState->DeltaTime, playerPhysics.Transform, velocity);

			// 인풋 재처리
			if (class UActorInputSyncComponent* pawnSyncComp = targetPlayer->GetInputSyncComp())
			{
				uint32 processedTick = 0;
				if (!gameState->ProcessedTicks.Contains(playerPhysics.SessionId))
				{
					UE_LOG(LogTemp, Warning, TEXT("세션 아이디 %i번 플레이어의 ProcessedTick 정보를 찾을 수 없습니다. 인풋 재처리를 실행하지 않습니다."));
					continue;
				}
				else
				{
					processedTick = gameState->ProcessedTicks[playerPhysics.SessionId];
				}

				pawnSyncComp->ReapplyLocalInputAfter(processedTick); // TODO: 보정용 인풋 재처리 시에는 애니메이션 업데이트 X
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("플레이어 폰에 대한 ActorInputSyncComponent를 찾지 못했습니다! 플레이어 캐릭터의 인풋을 재처리하지 못해 끊김이 발생할 수 있습니다."))
			}
		}
	}
}

void GameStateApplier::OnNewClientConnection_UEClient(uint64 clientSessionId, ANetHandler* netHandler)
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
