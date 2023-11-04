// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "SerializableData.h"
#include "PlayerPawn.h"

SD_PlayerState::SD_PlayerState(uint16 sessionId, APlayerPawn* player) : PlayerPhysics(Cast<AActor>(player))
{
	SessionId = sessionId;
	AnimState = player->GetCurrentAnimState();
	AnimStatus1D = player->GetCurrentAnimStatus();
	// TODO: Hp 등 각종 정보 전달
}