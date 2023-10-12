// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"

struct PacketHeader
{
	uint16 size; // 헤더를 포함한 패킷의 총 크기
	uint16 senderType; // 발신자 타입 (ExecType)
	uint64 senderId; // 발신자와 연동된 세션의 세션 id
	uint8 protocol; // 패킷 처리 방식
	uint16 id; // 패킷 id
};

enum PacketId : uint16
{
	DEFAULT,

	//// 클라이언트
	// 인풋
	GAME_INPUT,
	// 채팅 패킷
	CHAT_GLOBAL,

	//// 로직 서버
	GAME_STATE,

	//// 미들맨
	// 미들맨에서 로컬 호스트 세션의 ID를 발급받기 위해 사용
	SESSION_INFO,
	// 미들맨에 새로운 클라이언트 세션 연결/연결 해제 시 로직서버에게 알림
	SESSION_CONNECTED,
	SESSION_DISCONNECTED,
};

enum PacketProtocol : uint8
{
	NO_PROTOCOL,

	// 해당 프로토콜을 가진 패킷은 단일 세션에 대해 틱 당 한 개만 처리된다.
	CLIENT_ONCE_PER_TICK,

	// 해당 프로토콜을 가진 패킷은 단일 틱 당 여러 개 처리가 가능하다.
	CLIENT_ALLOW_MULTIPLE_PER_TICK,

	// 로직 서버가 처리해야 하는 패킷
	// 처리가 보장되며 순서는 큰 의미가 없다
	LOGIC_EVENT,

	// 미들맨 패킷
	// 처리가 보장되며 순서가 존재하지 않는다
	MIDDLEMAN_EVENT,
};