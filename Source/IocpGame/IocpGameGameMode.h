// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ExecType.h"
#include "IocpGameGameMode.generated.h"

UCLASS(minimalapi)
class AIocpGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIocpGameGameMode();

public:
	ExecType GameExecType; // TODO: ExecType 변경 가능하게 하는 기능
};



