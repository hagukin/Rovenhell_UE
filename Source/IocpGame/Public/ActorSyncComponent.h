// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RovenhellGameInstance.h"
#include "GameTickCounter.h"
#include "ActorSyncComponent.generated.h"

#define MAX_PHYSICS_HISTORY_SIZE 60 // 기록할 수 있는 최대 틱 히스토리 범위; 서버와 이 틱 크기만큼 연산이 차이가 나게 되면 끊김이 지속적으로 발생한다
// 이 길이를 지나치게 줄이면 위치 보정이 과도하게 잦아지고, 지나치게 늘리면 서버와 완전히 다른 위치임에도 올바른 위치에 있다고 판정할 확률이 높아진다

struct ActorPhysics
{
	FTransform transform;
	FVector Velocity;
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
	bool IsActorInSyncWith(const FTransform& Transform, const FVector& Velocity);
	void AdjustActorPhysics(float ServerDeltaTime, const FTransform& Transform, const FVector& Velocity); // 해당 틱에 해당 물리 정보를 갖도록 액터 정보 수정
	
private:
	bool IsValidPhysicsData(uint32 index, const FTransform& Transform, const FVector& Velocity); // 히스토리의 index번 물리 정보가 주어진 물리 정보와 일치하는지 반환한다 (boolean을 int8 형식으로)
	void MoveOne() { Head = (Head + 1) % MAX_PHYSICS_HISTORY_SIZE; Tail = (Tail + 1) % MAX_PHYSICS_HISTORY_SIZE; } // 헤드 및 테일을 1씩 이동시킨다

private:
	uint32 Head = 0;
	uint32 Tail = 0;
	TStaticArray<ActorPhysics, MAX_PHYSICS_HISTORY_SIZE> PhysicsHistory; // Circular Array로 사용

	bool StartTicking = false;
	const double ALLOWED_LOCATION_DIFFERENCE_WITH_SERVER = 10.0; // cm
	const uint32 IGNORE_RECENT_HISTORY_SIZE = (uint32)(DESIRED_SERVER_BROADCAST_TIME / DESIRED_DELTATIME); // 서버는 과거의 데이터를 발송하기 때문에, 지나치게 앞쪽에 위치한 히스토리들은 무시한다
	// 서버가 200ms 주기로 스테이트를 보내고 클라가 16ms 틱을 사용한다고 가정하자.
	// 이때 이 값은 대략 12가 사용되는데, 서버가 200ms(약 16ms*12)의 주기로 틱을 발송하기 때문에, 지난 200ms의 플레이어 틱 히스토리는 수신 스테이트와 비교하는 의미가 없다
	// 때문에 마지막 12개 만큼의 히스토리를 건너뛴다
};
