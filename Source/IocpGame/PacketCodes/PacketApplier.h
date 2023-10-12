// Copyright 2023 Haguk Kim
// Author: Haguk Kim

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

	/*UEClient*/
	TMap<uint8, FArchive> PacketFragments; // 서버의 데이터를 Gather하기 위해 사용
};
