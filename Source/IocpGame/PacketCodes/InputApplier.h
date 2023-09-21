// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"
#include "RovenhellGameInstance.h"

/**
 * 
 */
class IOCPGAME_API InputApplier : public PacketApplier
{
public:
	InputApplier();
	virtual ~InputApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler) override;
	bool ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
	bool ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
};
