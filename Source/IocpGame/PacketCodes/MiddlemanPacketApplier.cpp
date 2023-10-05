// Fill out your copyright notice in the Description page of Project Settings.


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
    const PacketHeader header = *((PacketHeader*)(packet->GetBuf()));
	bool applied = false;
	switch (header.id)
	{
	    case PacketId::SESSION_INFO:
	    {
		    applied = ApplySessionInfo(packet, netHandler);
		    break;
	    }
        case PacketId::SESSION_CONNECTED:
        {
            applied = ApplySessionConnection(packet, netHandler);
            break;
        }
        case PacketId::SESSION_DISCONNECTED:
        {
            applied = ApplySessionDisconnection(packet, netHandler);
            break;
        }
	    default:
        {
            UE_LOG(LogTemp, Warning, TEXT("MiddlemanPacketApplier가 처리할 수 없는 잘못된 패킷입니다."), header.id);
            break;
        }
	}
	return applied;
}

bool MiddlemanPacketApplier::ApplySessionInfo(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // 호스트 세션 id 초기화
    uint64 hostSessionId = ((PacketHeader*)(packet->GetBuf()))->senderId;
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
    netHandler->GetRovenhellGameInstance()->AddPlayerController(hostSessionId, UGameplayStatics::GetPlayerController(netHandler, 0));

    // TODO: 최초 GameState 동기화 전까지 이 플레이어의 인풋 막아두고 화면 상에 동기화 전의 상태를 렌더링하지 않기
    return true;
}

bool MiddlemanPacketApplier::ApplySessionConnection(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // 클라이언트 세션 추가
    uint64 clientSessionId = ((PacketHeader*)(packet->GetBuf()))->senderId;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i번 클라이언트 세션 연결"), clientSessionId));

    // TODO: 폰 생성 후 등록
    return true;
}

bool MiddlemanPacketApplier::ApplySessionDisconnection(TSharedPtr<RecvBuffer> packet, ANetHandler* netHandler)
{
    // 클라이언트 세션 추가
    uint64 clientSessionId = ((PacketHeader*)(packet->GetBuf()))->senderId;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%i번 클라이언트 세션 연결 해제"), clientSessionId));

    // TODO: 폰 삭제
    return true;
}
