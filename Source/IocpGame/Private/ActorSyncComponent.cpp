// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorSyncComponent.h"

// Sets default values for this component's properties
UActorSyncComponent::UActorSyncComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UActorSyncComponent::BeginPlay()
{
	Super::BeginPlay();
	HostTypeEnum hostType = Cast<URovenhellGameInstance>(GetWorld()->GetGameInstance())->GetExecType()->GetHostType();
	if (hostType == HostTypeEnum::CLIENT || hostType == HostTypeEnum::CLIENT_HEADLESS) StartTicking = true;
}

// Called every frame
void UActorSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (StartTicking)
	{
		uint32 tick = Cast<URovenhellGameInstance>(GetWorld()->GetGameInstance())->TickCounter->GetTick();
		FTransform currTransform = GetOwner()->GetTransform();
		FVector currVelocity = GetOwner()->GetRootComponent()->ComponentVelocity; // TODO: 점검 필요

		ActorPhysics actorPhysics = { currTransform, currVelocity, tick };

		if (!Tail)
		{
			Tail = new TList<ActorPhysics>(actorPhysics, nullptr);
			actorPhysics.Tick = FMath::Max<uint32>(actorPhysics.Tick - 1, 0); // 가상의 헤더 틱이 0 아래로 내려가지 않도록
			Head = new TList<ActorPhysics>(actorPhysics, Tail);
		}
		else if (Tail->Element.Tick >= tick)
		{
			// 최신 정보로 업데이트, 단 틱은 수정 X
			Tail->Element.transform = currTransform;
			Tail->Element.Velocity = currVelocity;
		}
		else
		{
			// 현재 틱까지 노드 제작해서 채운다
			for (uint32 t = Tail->Element.Tick + 1; t <= tick; ++t)
			{
				actorPhysics.Tick = t;
				Tail->Next = new TList<ActorPhysics>(actorPhysics, nullptr);
				Tail = Tail->Next;
			}
		}

		// 최대 개수 초과했을 경우 Head부터 순서대로 삭제한다
		if (Tail->Element.Tick - Head->Element.Tick > MAX_HISTORY_SIZE)
		{
			MoveHeadTo(Tail->Element.Tick - MAX_HISTORY_SIZE);
		}
	}
}

uint32 UActorSyncComponent::IsActorInSyncWith(uint32 Tick, const FTransform& Transform, const FVector& Velocity)
{
	TList<ActorPhysics>* Current = Head;
	if (!Current) return 0;
	while (Current->Next && Current->Element.Tick <= Tick + MAX_TICK_DIFF_ALLOWED)
	{
		if (Current->Element.Tick < Tick - MAX_TICK_DIFF_ALLOWED)
		{
			Current = Current->Next;
			continue;
		}

		// 물리 정보가 서버와 일치하는지 판단
		if (IsValidPhysicsData(Current, Transform, Velocity))
		{
			return Current->Element.Tick;
		}
		else
		{
			Current = Current->Next;
			continue;
		}
	}
	return 0;
}

void UActorSyncComponent::AdjustActorPhysics(float ServerDeltaTime, uint32 Tick, const FTransform& Transform, const FVector& Velocity)
{
	//// TESTING
	UE_LOG(LogTemp, Log, TEXT("테스트를 위해 Adjust 요청을 무시합니다"));
	return;




	FTransform newTransform = Transform;
	int64 tickDiff = Cast<URovenhellGameInstance>(GetOwner()->GetGameInstance())->TickCounter->GetTick() - Cast<URovenhellGameInstance>(GetOwner()->GetGameInstance())->TickCounter->GetServerTick();
	if (tickDiff < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("클라이언트 틱이 서버로부터 수신받은 틱보다 뒤에 있습니다. Dead Reckoning을 사용할 수 없습니다!"));
		GetOwner()->SetActorTransform(Transform);
		GetOwner()->GetRootComponent()->GetComponentVelocity() = Velocity;
		return;
	}

	// NOTE:
	// 언리얼 물리 엔진을 이용한 Server reconciliation을 이벤트 틱 내에서 수행하기가 어려운 관계로 
	// 속도가 동일하다는 가정 하에 Dead reckoning을 사용해 대략적인 위치를 추정해 현재 이 액터에 그 위치를 대입하는 방식을 사용한다.
	newTransform.SetLocation(Transform.GetLocation() + (Velocity * ServerDeltaTime * tickDiff)); // 서버 델타 사용 이유: 서버측 이벤트 틱의 인풋 처리 결과와 싱크를 맞추기 위함

	GetOwner()->GetRootComponent()->ComponentVelocity = Velocity;
	GetOwner()->SetActorTransform(newTransform);

	DrawDebugSphere(GetWorld(), newTransform.GetLocation(), 10, 26, FColor(255, 0, 0), false, 1.0f, 0, 1);
}

bool UActorSyncComponent::IsValidPhysicsData(TList<ActorPhysics>* Node, const FTransform& Transform, const FVector& Velocity)
{
	// 서버에서의 이동 연산 결과는 결국 클라이언트 틱에서의 연산결과 중 하나와 매우 근접하다 
	// (클라와 똑같은 정보를 처리하며 처리 rate만 다른 것이기 때문에)
	// 따라서 클라이언트의 틱n, n+1 에서의 위치 사이에 서버 틱이 위치하면 정상으로 간주한다.
	// 식: AB = AC + BC
	if (!Node->Next) return false;
	return FVector::Dist(Node->Element.transform.GetLocation(), Transform.GetLocation()) + FVector::Dist(Node->Next->Element.transform.GetLocation(), Transform.GetLocation()) == FVector::Dist(Node->Element.transform.GetLocation(), Node->Next->Element.transform.GetLocation()); // Velocity 고려 X
}

void UActorSyncComponent::MoveHeadTo(uint32 Tick)
{
	if (!Head || !(Head->Next))
	{
		UE_LOG(LogTemp, Error, TEXT("액터 히스토리를 조정하던 과정에서 예기치 못한 에러가 발생했습니다."));
		Head = nullptr;
		Tail = nullptr;
		// Potential memory leak
		return;
	}
	while (Head->Element.Tick < Tick)
	{
		TList<ActorPhysics>* temp = Head;
		Head = Head->Next;
		delete temp;
	}
}

