// Copyright 2023 Haguk Kim
// Author: Haguk Kim

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
	bool ApplySessionConnection_UEServer(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
	bool ApplySessionDisconnection_UEServer(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
};
