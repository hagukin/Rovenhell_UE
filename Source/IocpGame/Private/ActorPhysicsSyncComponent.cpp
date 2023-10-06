// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorPhysicsSyncComponent.h"

UActorPhysicsSyncComponent::UActorPhysicsSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UActorPhysicsSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	HostTypeEnum hostType = Cast<URovenhellGameInstance>(GetWorld()->GetGameInstance())->GetExecType()->GetHostType();
	if (hostType == HostTypeEnum::CLIENT || hostType == HostTypeEnum::CLIENT_HEADLESS)
	{
		StartTicking = true;
		for (int i = 0; i < MAX_PHYSICS_HISTORY_SIZE; ++i)
		{
			PhysicsHistory[i] = { GetOwner()->GetTransform(), GetOwner()->GetRootComponent()->ComponentVelocity };
		}
	}
}

void UActorPhysicsSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (StartTicking)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

		// 커서 이동
		MoveOnePhysicsHistoryCursor();
		AddCurrentPhysicsInfo();
	}
}

bool UActorPhysicsSyncComponent::IsActorInSyncWith(const FTransform& Transform, const FVector& Velocity)
{
	// 틱 히스토리 내부에 주어진 입력과 싱크가 맞는 기록이 있는지 확인한다
	uint32 currentIndex = PhysHead;
	for (uint32 i = 0; i < (uint32)MAX_PHYSICS_HISTORY_SIZE; ++i)
	{
		// 물리 정보가 서버와 일치하는 틱이 있는지 판정
		if(IsValidPhysicsData(currentIndex, Transform, Velocity))
		{
			return true;
		}
		currentIndex = (currentIndex + 1) % MAX_PHYSICS_HISTORY_SIZE;
	}
	// 여기까지도 일치하는 정보가 없을 경우 현재 액터 물리 정보를 추가한 후 마지막 대조
	uint32 prev = PhysTail;
	MoveOnePhysicsHistoryCursor();
	AddCurrentPhysicsInfo(); // 현재 물리 정보 추가
	return IsValidPhysicsData(prev, Transform, Velocity);
}

void UActorPhysicsSyncComponent::AdjustActorPhysics(float ServerDeltaTime, const FTransform& Transform, const FVector& Velocity)
{
	// 파라미터로 전달되는 서버 델타는 기존에 추측 항법 계산을 위해 사용했으나, 현재는 사용하지 않는다
	// 다만 삭제는 아직 하지 않고 일단 보류한다

	// 디버깅용
	/*DrawDebugPoint(GetWorld(), GetOwner()->GetTransform().GetLocation(), 5, FColor(255, 0, 0), true, 5.0);
	DrawDebugPoint(GetWorld(), Transform.GetLocation(), 5, FColor(0, 255, 0), true, 5.0);*/

	// 마지막으로 수신한 서버측 위치로 이동한다
	GetOwner()->SetActorTransform(Transform, false, nullptr, ETeleportType::None);
	GetOwner()->GetRootComponent()->ComponentVelocity = Velocity;
}

void UActorPhysicsSyncComponent::AddCurrentPhysicsInfo()
{
	ActorPhysics actorPhysics = { GetOwner()->GetTransform(), GetOwner()->GetRootComponent()->ComponentVelocity }; // TODO: Velocity 점검 필요
	PhysicsHistory[PhysTail] = actorPhysics; // 노드 생성
}

bool UActorPhysicsSyncComponent::IsValidPhysicsData(uint32 index, const FTransform& Transform, const FVector& Velocity)
{
	// 서버에서의 이동 연산 결과는 결국 클라이언트 틱에서의 연산결과 중 하나와 매우 근접하다 
	// (클라와 똑같은 정보를 처리하며 처리 rate만 다른 것이기 때문에)
	// 따라서 클라이언트의 틱n, n+1 에서의 위치 사이에 서버 틱이 위치하면 정상으로 간주한다.

	// 두 점 사이에 한 점이 있는지 판정
	double AB = FVector::Dist(PhysicsHistory[index].transform.GetLocation(), Transform.GetLocation());
	double BC = FVector::Dist(Transform.GetLocation(), PhysicsHistory[(index + 1) % MAX_PHYSICS_HISTORY_SIZE].transform.GetLocation());
	double AC = FVector::Dist(PhysicsHistory[index].transform.GetLocation(), PhysicsHistory[(index + 1) % MAX_PHYSICS_HISTORY_SIZE].transform.GetLocation());
	
	// 디버깅용
	/*if ((FMath::Abs(AB + BC - AC) <= ALLOWED_LOCATION_DIFFERENCE_WITH_SERVER))
		DrawDebugLine(GetWorld(), PhysicsHistory[index].transform.GetLocation(), PhysicsHistory[(index + 1) % MAX_PHYSICS_HISTORY_SIZE].transform.GetLocation(), FColor(255, 255, 0), true, -1.0, 0, 0.7);*/

	return (FMath::Abs(AB + BC - AC) <= ALLOWED_LOCATION_DIFFERENCE_WITH_SERVER); // 속도, 각속도 고려 X
}
