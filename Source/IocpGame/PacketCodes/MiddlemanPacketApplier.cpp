// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "MiddlemanPacketApplier.h"
#include "PlayerPawn.h"

class APlayerPawn;

MiddlemanPacketApplier::MiddlemanPacketApplier()
{
}

MiddlemanPacketApplier::~MiddlemanPacketApplier()
{
}

bool MiddlemanPacketApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
    if (!PacketApplier::Init(session, gameInstance)) return false;
    return true;
}

bool MiddlemanPacketApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, class ANetHandler* netHandler)
{
    const PacketHeader header = *(packet->GetHeader());
	bool applied = false;
	switch (header.type)
	{
	    case PacketType::SESSION_INFO:
	    {
		    applied = ApplySessionInfo(packet, netHandler);
		    break;
	    }
        case PacketType::SESSION_CONNECTED:
        {
            // 어떤 클라이언트의 연결 알림은 오직 로직 서버만 수신함
            // 그 후 로직 서버가 접속을 나머지 클라이언트에게 알림
            // 이는 로직서버에서 전처리를 한 후 (플레이어의 물리 세계와의 상호작용 이후) 최초 상태를 모든 클라이언트에게 발송하는 것이 안전하기 때문임.
            applied = ApplySessionConnection(packet, netHandler);
            break;
        }
        case PacketType::SESSION_DISCONNECTED:
        {
            applied = ApplySessionDisconnection(packet, netHandler);
            break;
        }
	    default:
        {
            UE_LOG(LogTemp, Warning, TEXT("MiddlemanPacketApplier가 처리할 수 없는 잘못된 패킷입니다."), header.type);
            break;
        }
	}
	return applied;
}

bool MiddlemanPacketApplier::ApplySessionInfo(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // 호스트 세션 id 초기화
    uint16 hostSessionId = packet->GetHeader()->senderId;
    Session->SetSessionId(hostSessionId);
    UE_LOG(LogTemp, Log, TEXT("이 세션의 id가 %i로 설정되었습니다."), hostSessionId);


    // TODO: 로직서버와 클라이언트 차별화
    // 현재는 디버깅의 편의를 위해 로직서버와 클라이언트 1번이 동일한 폰을 소유한 것으로 처리중임.
    
    // 호스트 폰의 Owner Session id 초기화
    APlayerPawn* playerPawn = Cast<APlayerPawn>(UGameplayStatics::GetPlayerPawn(netHandler, 0)); // 호스트 폰(0)
    if (!playerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("이 로컬 호스트 소유의 폰에 접근할 수 없습니다."));
        return false;
    }
    if (!playerPawn->SetOwnerSessionId(hostSessionId))
    {
        UE_LOG(LogTemp, Log, TEXT("이 로컬 호스트 폰의 세션 아이디 %i를 등록하는 데 실패했습니다."), hostSessionId);
        return false;
    }

    // 로컬 호스트 컨트롤러 등록
    APlayerPawn* hostPlayer = Cast<APlayerPawn>(UGameplayStatics::GetPlayerController(netHandler, 0)->GetPawn());
    if (!hostPlayer)
    {
        UE_LOG(LogTemp, Log, TEXT("이 로컬 호스트의 플레이어 폰을 찾는 데 실패했습니다."));
        return false;
    }
    netHandler->GetRovenhellGameInstance()->AddPlayer(hostSessionId, hostPlayer);

    // TODO: 최초 GameState 동기화 전까지 이 플레이어의 인풋 막아두고 화면 상에 동기화 전의 상태를 렌더링하지 않기
    return true;
}

bool MiddlemanPacketApplier::ApplySessionConnection(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // 클라이언트 세션 추가
    uint16 clientSessionId = packet->GetHeader()->senderId;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i번 클라이언트 세션 연결"), clientSessionId));

    // 플레이어 폰 추가
    TWeakObjectPtr<ANetActorSpawner> netSpawner = netHandler->GetRovenhellGameInstance()->GetNetActorSpawner();
    if (netSpawner != nullptr && netSpawner.IsValid())
    {
        APlayerPawn* player = netSpawner->SpawnNewPlayerPawn();
        return netHandler->GetRovenhellGameInstance()->AddPlayer(clientSessionId, player);
    }
    UE_LOG(LogTemp, Error, TEXT("NetSpawner를 찾지 못했습니다."));
    return false;
}

bool MiddlemanPacketApplier::ApplySessionDisconnection(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // 클라이언트 세션 추가
    uint16 clientSessionId = packet->GetHeader()->senderId;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i번 클라이언트 세션 연결 해제"), clientSessionId));

    // 플레이어 폰 삭제
    TWeakObjectPtr<ANetActorSpawner> netSpawner = netHandler->GetRovenhellGameInstance()->GetNetActorSpawner();
    if (netSpawner != nullptr && netSpawner.IsValid())
    {
        APlayerPawn* player = netHandler->GetRovenhellGameInstance()->GetPlayerOfOwner(clientSessionId);
        netHandler->GetRovenhellGameInstance()->RemovePlayer(clientSessionId);
        return netSpawner->Remove(player);
    }
    UE_LOG(LogTemp, Error, TEXT("NetSpawner를 찾지 못했습니다."));
    return false;
}

bool MiddlemanPacketApplier::ApplySessionDisconnection_UEClient(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // TODO: 현재는 클라, 서버 둘다 처리가 동일하므로 굳이 이 함수를 쓸 필요가 없음
    // 추후 필요에 따라 사용할 것
    return false;
}

bool MiddlemanPacketApplier::ApplySessionDisconnection_UEServer(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    return false;
}
