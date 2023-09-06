// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsApplier.h"
#include "MyUtility.h"

PhysicsApplier::PhysicsApplier()
{
}

PhysicsApplier::~PhysicsApplier()
{
}

bool PhysicsApplier::Init(TSharedPtr<NetSession> session)
{
	if (!PacketApplier::Init(session)) return false;
	return true;
}

bool PhysicsApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("패킷 발송자 id: %i 발신자 타입: %d 틱: %i 내용: %s"), ((PacketHeader*)(packet->GetBuf()))->senderId, ((PacketHeader*)(packet->GetBuf()))->senderType, ((PacketHeader*)(packet->GetBuf()))->tick, *MyBytesToString(packet->GetData(), (int32)packet->GetSize() - sizeof(PacketHeader))));
	return true;
}
