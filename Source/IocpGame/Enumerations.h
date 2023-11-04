// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"

enum HostTypeEnum : uint16
{
	NONE,
	CLIENT_HEADLESS,
	CLIENT,
	LOGIC_SERVER_HEADLESS,
	LOGIC_SERVER,
	MIDDLEMAN_SERVER,
};

enum ActionTypeEnum : uint8
{
	UNDEFINED,
	MOVE,
	JUMP,
};

enum AnimStateEnum : uint8
{
	NO_ANIM,
	IDLING,
	MOVING,
};