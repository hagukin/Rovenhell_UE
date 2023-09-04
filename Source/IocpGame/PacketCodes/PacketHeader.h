// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct PacketHeader
{
	uint16 size; // 헤더를 포함한 패킷의 총 크기
	uint16 senderType; // 발신자 타입 (ExecType)
	uint16 id; // 패킷 id
	uint32 tick; // 게임 틱 - 클라이언트: 로컬 게임 틱(서버에게 패킷 수신 받을 때마다 true 틱으로 갱신); 서버: 게임 틱 count
};

enum PacketId : uint8
{
	DEFAULT,
	CHAT_GLOBAL,
	CLIENT_EVENT,
};