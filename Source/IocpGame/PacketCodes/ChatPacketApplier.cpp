// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "ChatPacketApplier.h"
#include "PlayerPawn.h"
#include "MyUtility.h"

ChatPacketApplier::ChatPacketApplier()
{
}

ChatPacketApplier::~ChatPacketApplier()
{
}

bool ChatPacketApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
    if (!PacketApplier::Init(session, gameInstance)) return false;
    return true;
}

bool ChatPacketApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // TODO
	// FIXME
	// TESTING
	uint16 sessionId = packet->GetHeader()->senderId;
	if (sessionId == 1) return true; // 호스트 플레이어 제외

	// TestClient 이동 시뮬레이션
	APlayerPawn* playerPawn = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(sessionId);
	if (!playerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("세션 %i번 플레이어 폰이 없습니다."), sessionId);
		return false;
	}
	int randX = (int)FMath::RandBool() * (FMath::RandBool() ? 1 : -1);
	int randY = (int)FMath::RandBool() * (FMath::RandBool() ? 1 : -1);
	for (int i = 0; i < 6; ++i) // 0.0166 * 6 = 0.1 주기
	{
		playerPawn->Move_UEServer(FInputActionValue(FVector(randX, randY, 0)), 0.0166f);
	}
    return true;
}
