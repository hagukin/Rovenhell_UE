// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct PacketHeader
{
	uint16 size; // 헤더를 포함한 패킷의 총 크기
	uint16 senderType; // 발신자 타입 (ExecType)
	uint16 id; // 패킷 id
};

enum PacketId
{
	DEFAULT,
	CHAT_GLOBAL,
};
