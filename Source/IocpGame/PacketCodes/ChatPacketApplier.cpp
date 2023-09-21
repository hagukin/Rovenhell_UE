// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatPacketApplier.h"
#include "MyUtility.h"

ChatPacketApplier::ChatPacketApplier()
{
}

ChatPacketApplier::~ChatPacketApplier()
{
}

bool ChatPacketApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
    if (!PacketApplier::Init(session, gameInstance)) return false;
    return true;
}

bool ChatPacketApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
    // TODO
    return true;
}
