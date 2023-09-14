// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStateApplier.h"
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

bool GameStateApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	URovenhellGameInstance* gameInstance = Cast<URovenhellGameInstance>(GameInstance);
	if (!gameInstance) return false;

	bool applied = true;
	switch (gameInstance->GetExecType()->GetHostType())
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
			{
				applied &= ApplyPacket_UEClient(packet, deserializer);
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				applied &= ApplyPacket_UEServer(packet, deserializer);
				break;
			}
	}
    return applied;
}

bool GameStateApplier::ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	deserializer->Clear();
	SD_ActorPhysics* physicsData = new SD_ActorPhysics();
	deserializer->ReadDataFromBuffer(packet);
	deserializer->Deserialize((SD_Data*)physicsData);

	Cast<URovenhellGameInstance>(GameInstance)->TickCounter->SetServerTick_UEClient(physicsData->Tick); // 서버 틱과 동기화

	for (TActorIterator<ANetSyncPawn> iter(GameInstance->GetWorld()); iter; ++iter) // TODO: 플레이어 및 싱크 맞출 폰 여러개
	{
		FVector velocity(physicsData->XVelocity, physicsData->YVelocity, physicsData->ZVelocity);
		FVector angularVelocity(physicsData->XAngularVelocity, physicsData->YAngularVelocity, physicsData->ZAngularVelocity);
		if (!(*iter)->GetSyncComp()->IsActorInSyncWith(physicsData->Transform, velocity, angularVelocity))
		{
			(*iter)->GetSyncComp()->AdjustActorPhysics(physicsData->DeltaTime, physicsData->Transform, velocity, angularVelocity);
		}
		break;
	}
	return true;
}

bool GameStateApplier::ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	return true;
}
