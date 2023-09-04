// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"

/**
 * 
 */
class IOCPGAME_API ClientEventApplier : PacketApplier
{
public:
	ClientEventApplier();
	~ClientEventApplier();

	bool Init();
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet) override;
};
