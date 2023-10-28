// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "NetSyncPawn.h"
#include "NetPawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class IOCPGAME_API UNetPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UNetPawnMovementComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	ANetSyncPawn* GetNetSyncPawn();

protected:
	void ApplyNetPhysics(float DeltaTime);
	void ApplyGravity(float DeltaTime);
	FVector GetNewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const;
	const FVector GetNetPawnGravity() { return NetGravityZVelocity; }

private:
	FVector NetGravityZVelocity = { 0,0,-10000.0f }; // 중력은 가속운동이지만 네트워크 싱크를 위해 물리연산의 역연산이 현재 불가능한 관계로 빠른 등속도 운동으로 대체한다
};
