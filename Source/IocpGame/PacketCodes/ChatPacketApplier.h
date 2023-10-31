// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"

class ANetHandler;

/**
 * 
 */
class IOCPGAME_API ChatPacketApplier : public PacketApplier
{
public:
	ChatPacketApplier();
	virtual ~ChatPacketApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler) override;
};
