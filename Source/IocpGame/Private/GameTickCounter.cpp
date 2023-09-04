// Fill out your copyright notice in the Description page of Project Settings.


#include "GameTickCounter.h"

GameTickCounter::GameTickCounter()
{
}

void GameTickCounter::Tick(float DeltaTime)
{
	if (TickCount == GFrameCounter)
		return;
	TickCount = GFrameCounter; // GFrameCounter 매 프레임마다 Incremented
}

