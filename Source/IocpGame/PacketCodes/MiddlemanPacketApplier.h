// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"

/**
 * 
 */
class IOCPGAME_API MiddlemanPacketApplier : public PacketApplier
{
public:
	MiddlemanPacketApplier();
	virtual ~MiddlemanPacketApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler) override;
	bool ApplySessionInfo(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
	bool ApplySessionConnection(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
	bool ApplySessionDisconnection(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
};
