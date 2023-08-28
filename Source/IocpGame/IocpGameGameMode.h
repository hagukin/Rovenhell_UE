// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Enumerations.h"
#include "ExecType.h"
#include "IocpGameState.h"
#include "IocpGameGameMode.generated.h"

UCLASS(minimalapi)
class AIocpGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIocpGameGameMode();

public:
	// TODO: 현재는 테스트의 편의성을 위해 런타임에 위젯BP 상에서 ExecType을 결정하지만
	// 추후 이 부분에 대한 구조적 수정이 필요함
	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsClient() { GameExecType->SetHostType(HostTypeEnum::CLIENT); }

	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsClientHeadless() { GameExecType->SetHostType(HostTypeEnum::CLIENT_HEADLESS); }

	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsLogicServer() { GameExecType->SetHostType(HostTypeEnum::LOGIC_SERVER); }

	UFUNCTION(BlueprintCallable, Category = "ExecType")
	void SetAsLogicServerHeadless() { GameExecType->SetHostType(HostTypeEnum::LOGIC_SERVER_HEADLESS); }

public:
	const TSharedPtr<ExecType> GetExecType() { return GameExecType; }

private:
	TSharedPtr<ExecType> GameExecType;
};



