// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class IOCPGAME_API GameUtilities
{
public:
	GameUtilities() {};
	~GameUtilities() {};

	static double IsVectorCCW2D(FVector v1, FVector v2)
	{
		// 시작점이 두 벡터의 X, Y 값의 CCW 여부를 반환한다
		// 양수: 반시계, 음수: 시계, 0: 직선
		return (v1.X * v2.Y) - (v2.X * v1.Y);
	}
};
