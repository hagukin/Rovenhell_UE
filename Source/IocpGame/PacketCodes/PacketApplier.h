// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../NetCodes/NetBuffer.h"
#include "../NetCodes/NetSession.h"
#include "SerializeManager.h"

/**
 * 
 */
class IOCPGAME_API PacketApplier
{
public:
	PacketApplier() {}
	virtual ~PacketApplier() {}

	virtual bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
	{ 
		if (!session || !gameInstance) return false;
		Session = session;
		GameInstance = gameInstance;
		return true; 
	}
	virtual bool ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler) abstract;

protected:
	TSharedPtr<NetSession> Session = nullptr;
	UGameInstance* GameInstance = nullptr;
};
