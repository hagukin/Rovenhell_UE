// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStateApplier.h"
#include "PlayerPawn.h"
#include "../IocpGameCharacter.h"

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

bool GameStateApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
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
				applied &= ApplyPacket_UEServer(packet, netHandler);
				break;
			}
	}
    return applied;
}

bool GameStateApplier::ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
	netHandler->GetDeserializerShared()->Clear();
	SD_ActorPhysics* physicsData = new SD_ActorPhysics();
	netHandler->GetDeserializerShared()->ReadDataFromBuffer(packet);
	netHandler->GetDeserializerShared()->Deserialize((SD_Data*)physicsData);

	Cast<URovenhellGameInstance>(GameInstance)->TickCounter->SetServerTick_UEClient(physicsData->Tick); // 서버 틱과 동기화

	for (TActorIterator<ANetSyncPawn> iter(GameInstance->GetWorld()); iter; ++iter) // TODO: 플레이어 외에도 싱크 맞출 오브젝트에 대해
	{
		FVector velocity(physicsData->XVelocity, physicsData->YVelocity, physicsData->ZVelocity);
		FVector angularVelocity(physicsData->XAngularVelocity, physicsData->YAngularVelocity, physicsData->ZAngularVelocity);
		if (!(*iter)->GetPhysicsSyncComp()->IsActorInSyncWith(physicsData->Transform, velocity, angularVelocity))
		{
			// 서버에서 수신받은 물리 정보를 적용한다
			(*iter)->GetPhysicsSyncComp()->AdjustActorPhysics(physicsData->DeltaTime, physicsData->Transform, velocity, angularVelocity);

			APlayerPawn* playerPawn = Cast<APlayerPawn>(*iter);
			if (playerPawn) // Cast 가능한 경우에만 진입 (typecheck)
			{
				if (class UActorInputSyncComponent* pawnSyncComp = playerPawn->GetInputSyncComp())
				{
					pawnSyncComp->ReapplyLocalInputAfter(physicsData->ProcessedTick); // 누적 인풋 재처리
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("플레이어 폰에 대한 ActorInputSyncComponent를 찾지 못했습니다! 플레이어 캐릭터의 인풋을 재처리하지 못해 끊김이 발생할 수 있습니다."))
				}
			}
		}
		break;
	}
	return true;
}

bool GameStateApplier::ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
	return true;
}
