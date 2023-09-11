// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetPawnMovementComponent.h"
#include "NetHandler.h"
#include "NetPlayerMovementComponent.generated.h"

class APlayerPawn;
/**
 * 
 */
UCLASS()
class IOCPGAME_API UNetPlayerMovementComponent : public UNetPawnMovementComponent
{
	GENERATED_BODY()
	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	APlayerPawn* GetPlayerOwner();
	ANetHandler* GetNetHandler();
};
