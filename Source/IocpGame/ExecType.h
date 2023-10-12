// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Enumerations.h"

/**
 * 실행 타입을 나타낸다.
 * 기본값: 클라이언트
 */
class IOCPGAME_API ExecType
{
public:
	ExecType();
	ExecType(HostTypeEnum type) : HostType(type) {};
	~ExecType();

	HostTypeEnum GetHostType() { return HostType; }
	void SetHostType(HostTypeEnum type) { HostType = type; }

private:
	HostTypeEnum HostType;
};
