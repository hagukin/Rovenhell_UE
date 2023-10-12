// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "NetPawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class IOCPGAME_API UNetPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
