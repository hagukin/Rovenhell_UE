// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RovenhellGameInstance.h"
#include "GameTickCounter.h"
#include "InputActionValue.h"
#include "ActorPhysicsSyncComponent.generated.h"

#define MAX_PHYSICS_HISTORY_SIZE 60 // 기록할 수 있는 최대 틱 히스토리 범위

struct ActorPhysics
{
	FTransform transform;
	FVector Velocity;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class IOCPGAME_API UActorPhysicsSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorPhysicsSyncComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool IsActorInSyncWith(const FTransform& Transform, const FVector& Velocity);
	void AdjustActorPhysics(float ServerDeltaTime, const FTransform& Transform, const FVector& Velocity); // 해당 틱에 해당 물리 정보를 갖도록 액터 정보 수정
	void AddCurrentPhysicsInfo(); // Tail에 액터 정보를 덮어쓴다

protected:
	virtual void BeginPlay() override;
	bool IsValidPhysicsData(uint32 index, const FTransform& Transform, const FVector& Velocity); // 히스토리의 index번 물리 정보가 주어진 물리 정보와 일치하는지 반환한다 (boolean을 int8 형식으로)
	void MoveOnePhysicsHistoryCursor() { PhysHead = (PhysHead + 1) % MAX_PHYSICS_HISTORY_SIZE; PhysTail = (PhysTail + 1) % MAX_PHYSICS_HISTORY_SIZE; } // 헤드 및 테일을 1씩 이동시킨다

protected:
	uint32 PhysHead = 0; // 가장 오래된 정보
	uint32 PhysTail = MAX_PHYSICS_HISTORY_SIZE - 1; // 가장 뒷번 인덱스로 초기화; 가장 최근 정보
	TStaticArray<ActorPhysics, MAX_PHYSICS_HISTORY_SIZE> PhysicsHistory; // Circular Array로 사용

private:
	bool StartTicking = false;
	const double ALLOWED_LOCATION_DIFFERENCE_WITH_SERVER = 3.0; // 단위 cm; 서버 보정이 지나치게 빈번하게 일어난다면 허용 오차 크기를 늘리는 것을 고려하자.
};