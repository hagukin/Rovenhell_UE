// Copyright 2023 Haguk Kim
// Author: Haguk Kim

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
