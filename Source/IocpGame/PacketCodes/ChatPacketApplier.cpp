// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatPacketApplier.h"
#include "MyUtility.h"

ChatPacketApplier::ChatPacketApplier()
{
}

ChatPacketApplier::~ChatPacketApplier()
{
}

bool ChatPacketApplier::Init()
{
    return true;
}

bool ChatPacketApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("채팅 %d : %s"), ((PacketHeader*)(packet->GetBuf()))->senderType, *MyBytesToString(packet->GetData(), (int32)packet->GetSize() - sizeof(PacketHeader))));
    return true;
}
