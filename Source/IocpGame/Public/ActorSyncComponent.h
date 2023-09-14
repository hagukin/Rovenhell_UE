// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RovenhellGameInstance.h"
#include "GameTickCounter.h"
#include "ActorSyncComponent.generated.h"

#define MAX_PHYSICS_HISTORY_SIZE 240 // 기록할 수 있는 최대 틱 히스토리 범위; 서버와 이 틱 크기만큼 연산이 차이가 나게 되면 끊김이 지속적으로 발생한다
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
	bool IsActorInSyncWith(const FTransform& Transform, const FVector& Velocity, const FVector& AngularVelocity);
	void AdjustActorPhysics(float ServerDeltaTime, const FTransform& Transform, const FVector& Velocity, const FVector& AngularVelocity); // 해당 틱에 해당 물리 정보를 갖도록 액터 정보 수정
	
private:
	bool IsValidPhysicsData(uint32 index, const FTransform& Transform, const FVector& Velocity, const FVector& AngularVelocity); // 히스토리의 index번 물리 정보가 주어진 물리 정보와 일치하는지 반환한다 (boolean을 int8 형식으로)
	void MoveOne() { Head = (Head + 1) % MAX_PHYSICS_HISTORY_SIZE; Tail = (Tail + 1) % MAX_PHYSICS_HISTORY_SIZE; } // 헤드 및 테일을 1씩 이동시킨다

private:
	uint32 Head = 0;
	uint32 Tail = 0;
	TStaticArray<ActorPhysics, MAX_PHYSICS_HISTORY_SIZE> PhysicsHistory; // Circular Array로 사용

	bool StartTicking = false;
	const double ALLOWED_LOCATION_DIFFERENCE_WITH_SERVER = 10; // 단위 cm; 
	// NOTE: 
	// 대체적으로 오차는 0이지만 slope나 추락과 같이 clock cycle이 높은 물리연산에 대해서 약간의 오차가 발생한다
	// 원인은 이 서버에서 패킷을 모아서 처리하는 과정에서 누락되는 연산들이 있기 때문인 것으로 생각된다.
	// 정확히는 누락된다기 보다, 언리얼 물리엔진보다 더 빠른 주기로 동일한 결과를 도출하는 게 불가능한 연산들이다.
	// 완전히 물리 엔진단까지 접근할 수 있거나 직접 이벤트 틱에서 도는 물리엔진을 구현하면 해결이 가능할 지도 모르지만
	// 전자는 오히려 더 많은 문제를 일으킬 수 있고, 후자는 내장된 물리엔진에 비해 성능이 훨씬 저하될 우려가 있다.
	// 따라서 선택한 타협안이 어느 정도의 물리적 거리 오차를 허용하는 방식이다.
};
