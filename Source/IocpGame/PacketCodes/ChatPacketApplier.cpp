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
	// 채팅 패킷을 처리하는 용도로 제작된 함수이지만 현재는 임시로 더미클라이언트들의 가상인풋을 처리하는 용도로 사용중임.
	uint16 sessionId = packet->GetHeader()->senderId;
	if (sessionId == 1) return true; // 호스트 플레이어 제외

	// TestClient 이동 시뮬레이션
	APlayerPawn* playerPawn = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(sessionId);
	if (!playerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("세션 %i번 플레이어 폰이 없습니다."), sessionId); 
		// NOTE: 테스트 클라이언트에는 굳이 고유 id 할당 기능을 안 만들어뒀기 때문에, 
		// 한 번 다른 클라이언트 세션의 disconnect 이후 테스트 클라이언트 접속 시 이 부분에서 로그가 떠도 정상이다
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
