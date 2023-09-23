// Fill out your copyright notice in the Description page of Project Settings.

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

	static double IsVectorCCW(double x1, double y1, double x2, double y2, double x3, double y3)
	{
		// 시작점이 x1, y1인 두 벡터의 CCW 여부를 반환한다
		// 양수: 반시계, 음수: 시계, 0: 직선
		return (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
	}
};
