// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsApplier.h"
#include "MyUtility.h"
#include "RovenhellGameInstance.h"

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
	

	return true;
}
