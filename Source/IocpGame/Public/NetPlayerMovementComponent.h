// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetPawnMovementComponent.h"
#include "NetHandler.h"
#include "NetPlayerMovementComponent.generated.h"

// 단일 인풋에 대한 데이터
typedef struct MoveInputData
{
	FVector MoveVector;
	float DeltaTime;
};

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

	void BeginTick();
	void EndTick();

	// 물리 연산
	// 주의: PawnMovement에 기본적으로 구현된 함수들(AddInputVector 등)은 우리의 넷코드와 호환되지 않기 때문에 사용하지 않는다
	void AddMovementData(FVector MoveVector, float DeltaTime); // 콜스택: InputApplier -> Move

private:
	// 이번 틱에 처리해야 하는 Move 인풋(들)의 델타 타임
	// 로직 서버의 경우 단일 폰에 대한 인풋 여러 번을 한번에 처리해야 하므로 Array의 크기가 1 이상일 수 있다.
	// 이 경우 어레이의 크기를 이용해 이번 틱에 처리되야 하는 총 Move 인풋의 개수를 구하기 때문에
	// TickComponent가 처리되는 동안에는 개수를 늘이거나 줄여서는 안된다
	TArray<MoveInputData> MoveDatas;

	// 회전
	FVector DesiredDirection;
	float TurnLeftYaw = -500.0f;
	float TurnRightYaw = 500.0f;
};
