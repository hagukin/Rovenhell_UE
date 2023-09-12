// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RovenhellGameInstance.h"
#include "ActorSyncComponent.generated.h"

struct ActorPhysics
{
	FTransform transform;
	FVector Velocity;
	uint32 Tick;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IOCPGAME_API UActorSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorSyncComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	uint32 IsActorInSyncWith(uint32 Tick, const FTransform& Transform, const FVector& Velocity); // 해당 물리 정보와 오차가 크지 않은 값을 해당 틱 부근에서 발견했을 경우 발견한 틱 번호를 반환한다. 찾지 못했을 경우 0을 반환한다.
	void AdjustActorPhysics(float ServerDeltaTime, uint32 Tick, const FTransform& Transform, const FVector& Velocity); // 해당 틱에 해당 물리 정보를 갖도록 액터 정보 수정
	
private:
	double GetDifference(TList<ActorPhysics>* Node, const FTransform& Transform, const FVector& Velocity); // Node 정보와 주어진 값의 오차를 구한다
	void MoveHeadTo(uint32 Tick); // Tick번 노드로 헤더를 옮기고, 기존 헤더에서 Tick번 노드 사이에 있는 모든 원소는 해제한다 // TODO: 해제 대신 풀 제작
private:
	TList<ActorPhysics>* Head = nullptr;
	TList<ActorPhysics>* Tail = nullptr;

	bool StartTicking = false;

	const int32 MAX_TICK_DIFF_ALLOWED = 8; // 서버틱과의 허용 오차틱 (+-)
	const double MAX_PHYSICS_DIFF_ALLOWED = 10.0; // 서버와의 물리 오차 허용 (cm)
	const uint32 MAX_HISTORY_SIZE = 100; // 기록할 수 있는 최대 틱 히스토리 범위; Head Tick과 Tail Tick의 차이는 이 값을 넘지 않는다
};
