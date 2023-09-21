// Fill out your copyright notice in the Description page of Project Settings.


#include "MiddlemanPacketApplier.h"

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
    // TEMP
    // deserialize middleman info
    // 패킷 헤더 senderId에 이 세션에게 할당된 id를 넣어 보낸다
    Session->SetSessionId(((PacketHeader*)(packet->GetBuf()))->senderId); // 세션id 설정
    UE_LOG(LogTemp, Log, TEXT("이 세션의 id가 %i로 설정되었습니다."), ((PacketHeader*)(packet->GetBuf()))->senderId);
    return true;
}