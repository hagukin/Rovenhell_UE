// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum ExecTypeEnum
{
	CLIENT_HEADLESS,
	CLIENT,
	SERVER_HEADLESS,
	SERVER
};

/**
 * 실행 타입을 나타낸다.
 * 기본값: 클라이언트
 */
class IOCPGAME_API ExecType
{
public:
	ExecType();
	ExecType(ExecTypeEnum type) : HostType(type) {};
	~ExecType();

	ExecTypeEnum GetHostType() { return HostType; }

private:
	ExecTypeEnum HostType;
};
