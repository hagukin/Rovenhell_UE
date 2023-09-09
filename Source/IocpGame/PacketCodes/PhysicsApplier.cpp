// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsApplier.h"
#include "MyUtility.h"

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

bool PhysicsApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet)
{
	// TEMP
	if (((PacketHeader*)(packet->GetBuf()))->id == PacketId::GAME_STATE)
	{
		URovenhellGameInstance* gameInstance = Cast<URovenhellGameInstance>(GameInstance);
		if (gameInstance->GetExecType()->GetHostType() == HostTypeEnum::CLIENT || gameInstance->GetExecType()->GetHostType() == HostTypeEnum::CLIENT_HEADLESS)
		{
			gameInstance->TickCounter->SetServerTick_UEClient(((PacketHeader*)(packet->GetBuf()))->tick); // 서버 틱과 동기화
		}
	}

	return true;
}
