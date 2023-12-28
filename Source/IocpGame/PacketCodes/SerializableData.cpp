// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "SerializableData.h"
#include "PlayerPawn.h"

SD_PlayerState::SD_PlayerState(uint16 sessionId, APlayerPawn* player) : PlayerPhysics(Cast<AActor>(player))
{
	SessionId = sessionId;
	AnimState = player->GetCurrentAnimState();

	// AnimState status delta 추출
	AnimDelta1D = player->GetCurrentAnimStatus() - player->GetSavedAnimStatus();
	player->SaveCurrentAnimStatus(); // 다음번 델타 추출을 위해 현재 값을 기록

	// TODO: Hp 등 각종 정보 전달
}