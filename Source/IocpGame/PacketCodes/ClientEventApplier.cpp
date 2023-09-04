// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientEventApplier.h"
#include "MyUtility.h"

ClientEventApplier::ClientEventApplier()
{
}

ClientEventApplier::~ClientEventApplier()
{
}

bool ClientEventApplier::Init()
{
	return true;
}

bool ClientEventApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("호스트 타입 %d 틱 %i : %s"), ((PacketHeader*)(packet->GetBuf()))->senderType, ((PacketHeader*)(packet->GetBuf()))->tick, *MyBytesToString(packet->GetData(), (int32)packet->GetSize() - sizeof(PacketHeader))));
	return true;
}
