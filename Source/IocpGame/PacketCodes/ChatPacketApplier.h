// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"

/**
 * 
 */
class IOCPGAME_API ChatPacketApplier : PacketApplier
{
public:
	ChatPacketApplier();
	~ChatPacketApplier();

	bool Init();
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet) override;
};
