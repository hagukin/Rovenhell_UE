// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Templates/SharedPointer.h"
#include "../ExecType.h"
#include "RovenhellGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class IOCPGAME_API URovenhellGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	URovenhellGameInstance();

	const TSharedPtr<ExecType> GetExecType() { return GameExecType; }

	// TODO: 런타임 이전에 호스트 타입을 정할 수 있도록 외부 장치 추가
	// 현재는 편의상 런타임에 유저가 결정 가능
	// 단 현재 방식으로는 Reflection이 Enum을 감지하지 못하기 때문에 수동으로 값을 입력해야 함
	UFUNCTION(BlueprintCallable)
	void SetHostTypeAs(uint8 hostType) { GameExecType->SetHostType((HostTypeEnum)hostType); }

private:
	TSharedPtr<ExecType> GameExecType = nullptr;
};
