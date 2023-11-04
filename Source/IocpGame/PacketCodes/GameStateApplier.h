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
	enum ConnectionStatus
	{
		NONEXISTENT, // 존재하지 않는 세션
		CONNECTED, // 접속됨/연결 처리해주어야함
		DISCONNECTED, // 연결 해제 처리해주어야함
	};

public:
	GameStateApplier();
	virtual ~GameStateApplier();

	bool Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance);
	bool ApplyPacket(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler) override;
	bool ApplyPacket_UEClient(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler);
	void ApplyGameState_UEClient(SD_GameState* gameState, ANetHandler* netHandler);

	void ApplyPlayerPhysics_UEClient(SD_GameState* gameState, const uint16 playerSessionId, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler); // 호스트 폰의 경우 클라와 서버의 싱크를 확인하고 필요서 PhysicsSyncComp로 보정한다; 퍼펫의 경우 InterpComp로 두 시점 사이의 움직임을 부드럽게 선형 보간 처리한다.
	void ApplyPlayerPhysicsOfHost_UEClient(APlayerPawn* hostPlayer, SD_GameState* gameState, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler);
	void ApplyPlayerPhysicsOfPuppet_UEClient(APlayerPawn* puppetPlayer, SD_GameState* gameState, const SD_PawnPhysics& playerPhysics, ANetHandler* netHandler);

	void ApplyPlayerAnimation_UEClient(SD_PlayerState* playerState, ANetHandler* netHandler);
	void ApplyPlayerAnimationOfHost_UEClient(APlayerPawn* hostPlayer, SD_PlayerState* playerState, ANetHandler* netHandler);
	void ApplyPlayerAnimationOfPuppet_UEClient(APlayerPawn* puppetPlayer, SD_PlayerState* playerState, ANetHandler* netHandler);

	void OnNewClientConnection_UEClient(uint16 clientSessionId, ANetHandler* netHandler);
	void OnNewClientDisconnection_UEClient(uint16 clientSessionId, ANetHandler* netHandler);
	void CheckForNewConnection_UEClient(uint16 sessionId, ANetHandler* netHandler); // 로직서버의 세션 정보를 싱크를 맞춘다; true일 경우 기존에 있는 플레이어 정보이거나 혹은 새로 접속한 플레이어 정보, false는 해당 플레이어의 Disconnect를 의미

private:
	TMap<uint16, ConnectionStatus> IsPlayerConnected;
};
