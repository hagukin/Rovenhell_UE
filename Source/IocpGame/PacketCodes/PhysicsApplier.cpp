// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsApplier.h"
#include "MyUtility.h"
#include "../IocpGameCharacter.h"

PhysicsApplier::PhysicsApplier()
{
}

PhysicsApplier::~PhysicsApplier()
{
}

bool PhysicsApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
	if (!PacketApplier::Init(session, gameInstance)) return false;
	return true;
}

bool PhysicsApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
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

bool PhysicsApplier::ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	return true;
}

bool PhysicsApplier::ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	deserializer->Clear();
	SD_Transform* transformData = new SD_Transform();
	deserializer->ReadDataFromBuffer(packet);
	deserializer->DeserializeTransform(transformData);
	UE_LOG(LogTemp, Warning, TEXT("%f %f %f / %f %f %f / %f %f %f"), transformData->Transform.GetLocation().X, transformData->Transform.GetLocation().Y, transformData->Transform.GetLocation().Z, transformData->Transform.GetRotation().X, transformData->Transform.GetRotation().Y, transformData->Transform.GetRotation().Z);

	// TODO: Level Streaming
	/*TArray<ULevelStreaming*> streamedLevels = GameInstance->GetWorld()->GetStreamingLevels();
	for (ULevelStreaming* streamLevel : streamedLevels)
	{
		ULevel* level = streamLevel->GetLoadedLevel();
		if (!level) continue;
		for (AActor* actor : level->Actors)
		{
		}
	}*/

	// 테스트
	// 애니메이션 적용 X
	for (TActorIterator<AIocpGameCharacter> iter(GameInstance->GetWorld()); iter; ++iter)
	{
		(*iter)->SetActorTransform(transformData->Transform);
	}

	return true;
}
