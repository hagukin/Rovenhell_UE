// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketApplier.h"
#include "RovenhellGameInstance.h"

/**
 * 
 */
class IOCPGAME_API GameStateApplier : public PacketApplier
{
public:
	GameStateApplier();
	virtual ~GameStateApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler) override;
	bool ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
	void ApplyPhysics_UEClient(SD_GameState* gameState, class ANetHandler* netHandler); // 패킷 중 피직스 관련 정보들을 처리하는 함수
	bool ApplyPacket_UEServer(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler);
};
