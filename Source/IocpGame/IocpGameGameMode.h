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
	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsClient() { GameExecType = ExecTypeEnum::CLIENT; }

	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsClientHeadless() { GameExecType = ExecTypeEnum::CLIENT_HEADLESS; }

	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsLogicServer() { GameExecType = ExecTypeEnum::SERVER; }

	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsLogicServerHeadless() { GameExecType = ExecTypeEnum::SERVER_HEADLESS; }

	ExecType GameExecType; // TODO: ExecType 변경 가능하게 하는 기능
};



