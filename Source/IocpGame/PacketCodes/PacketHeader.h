// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct PacketHeader
{
	uint16 size; // 헤더를 포함한 패킷의 총 크기
	uint16 senderType; // 발신자 타입 (ExecType)
	uint64 senderId; // 발신자와 연동된 세션의 세션 id
	uint8 protocol; // 패킷 처리 방식
	uint16 id; // 패킷 id
	uint32 tick; // 게임 틱 - 클라이언트: 로컬 게임 틱(서버에게 패킷 수신 받을 때마다 true 틱으로 갱신); 서버: 게임 틱 count
	float deltaTime; // 패킷 발송 시점에서의 델타타임
};

enum PacketId : uint16
{
	DEFAULT,

	//// 클라이언트
	// 인풋
	GAME_INPUT,
	// 채팅 패킷
	CHAT_GLOBAL,

	// 액터의 물리적 상태 변화
	ACTOR_PHYSICS,

	//// 로직 서버
	GAME_STATE,

	//// 미들맨
	// 미들맨에서 생성한 이 호스트 세션의 정보
	SESSION_INFO,
};

enum PacketProtocol : uint8
{
	NO_PROTOCOL,
	// 수신 시점에 처리되는 패킷
	// 수신받은 이상 처리가 보장된다
	// 단 최소한의 처리 순서 보장이 이루어지며 항상 보장되진 않는다 (즉 네트워크 상황에 따라 더 나중 틱의 정보가 먼저 처리될 수도 있다)
	CLIENT_EVENT_ON_RECV,

	// 반드시 발송한 틱 순서에 처리되어야 하는 패킷 (순서를 보장받아야 하는 패킷)
	// 만약 이미 해당 틱의 처리 순서가 지났다면 다음과 같은 방식들 중 선택이 가능하다
	// 1) 과거 기록을 바탕으로 틱 재계산 (FPS에서 사용하는 방식) -> 언리얼 물리연산 특성 상 사용하기 어려움, 구현하더라도 부하가 큼
	// 2) 해당 패킷 무시 -> CLIENT_EVENT_ON_TICK_STRICT
	// 3) 늦었더라도 현재 순서에서 동일한 요청 시행 (ON_RECV와 동일하게 처리) -> CLIENT_EVENT_ON_TICK_LOOSE
	CLIENT_EVENT_ON_TICK_STRICT,
	CLIENT_EVENT_ON_TICK_LOOSE,

	// 로직 서버 패킷
	// 처리가 보장되며 순서는 큰 의미가 없다
	LOGIC_EVENT,

	// 미들맨 패킷
	// 처리가 보장되며 순서가 존재하지 않는다
	MIDDLEMAN_EVENT,
};