// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"

/**
 * 
 */
class IOCPGAME_API ChatPacketApplier : public PacketApplier
{
public:
	ChatPacketApplier();
	virtual ~ChatPacketApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler) override;
};
