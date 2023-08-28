// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NetCodes/NetBuffer.h"

/**
 * 
 */
class IOCPGAME_API PacketApplier
{
public:
	virtual bool Init() { return true; }
	virtual bool ApplyPacket(TSharedPtr<RecvBuffer> packet) abstract;
};
