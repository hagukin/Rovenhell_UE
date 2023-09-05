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

	// 채팅 패킷
	CHAT_GLOBAL,

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
};