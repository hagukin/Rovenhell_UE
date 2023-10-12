// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"
#include "RovenhellGameInstance.h"

class ANetHandler;

/**
 * 
 */
class IOCPGAME_API GameStateApplier : public PacketApplier
{
public:
	GameStateApplier();
	virtual ~GameStateApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler) override;
	bool ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler);
	void ApplyPhysicsAndSyncPlayers_UEClient(SD_GameState* gameState, ANetHandler* netHandler); // 패킷 중 피직스 관련 정보들을 처리하고, 현재 접속한 플레이어들의 싱크를 로직서버와 맞춘다
	void OnNewClientConnection_UEClient(uint64 clientSessionId, ANetHandler* netHandler);
};
