// Copyright 2023 Haguk Kim
// Author: Haguk Kim


#include "NetSyncPawn.h"
#include "GameUtilities.h"
#include "NetPawnInterpComponent.h"

#define MAX_MOVE_PREDICTION_CYCLE_PER_TICK 100
#define MAX_MOVE_PREDICTION_QUEUE_SIZE 3 // 이 값을 너무 크게 잡으면 서버 위치와 클라에 표시되는 위치가 심각하게 차이가 발생하게 됨; 클라 위치가 (서버 패킷 송신주기 * 큐 크기)ms 차이난다고 생각하면 편함; 5 이하로 설정하는 것을 권장
#define DELTATIME_MULTIPLIER_DELTA_ON_QUEUE_OVERFLOW 0.25f // 큐가 허용치를 초과했을 때 매 틱마다 증가시킬 델타타임 배수값 / 큐가 허용치 내로 돌아왔을 때 매 틱마다 감소시킬 델타타임 배수값

UNetPawnInterpComponent::UNetPawnInterpComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UNetPawnInterpComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UNetPawnInterpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bHasBegunMovePrediction) return; // 로직서버의 경우 state packet을 받을 일이 없기 때문에 이 라인을 넘어가지 않음
	TimeSinceLastNewTransform += DeltaTime;

	if (QueueSize >= MAX_MOVE_PREDICTION_QUEUE_SIZE)
	{
		// 클라의 게임 틱 주기보다 서버의 스테이트 발송 주기가 월등히 느릴 경우 일어날 수 있다
		// 혹은 차이가 크지 않더라도 누적될 경우 일어날 수 있다
		DeltaTimeMultiplier += DELTATIME_MULTIPLIER_DELTA_ON_QUEUE_OVERFLOW; // 누적 발생 시 배수를 증가시킨다
		UE_LOG(LogTemp, Warning, TEXT("MovePrediction 큐가 적정 크기를 초과했습니다. (size: %i) 게임서버의 발송 주기가 느려졌을 가능성이 있습니다. 델타타임 배수: %f"), QueueSize, DeltaTimeMultiplier);
	}
	else
	{
		if (DeltaTimeMultiplier > 1.0f)
		{
			DeltaTimeMultiplier = FMath::Max(1.0f, DeltaTimeMultiplier - DELTATIME_MULTIPLIER_DELTA_ON_QUEUE_OVERFLOW); // 큐 크기가 허용치 내 인경우 기존에 배수가 증가되어있던 상태라면 배수를 감소시킨다
		}
	}

	float deltaTime = DeltaTime * DeltaTimeMultiplier; // 배수 적용
	int cycleCount = 0;
	while (deltaTime > KINDA_SMALL_NUMBER)
	{
		deltaTime = ApplyMovePrediction(deltaTime);
		cycleCount++;
		if (cycleCount > MAX_MOVE_PREDICTION_CYCLE_PER_TICK)
		{
			// DeltaTime 값이 매우 크거나 각 MovePrediction의 Timespan들이 매우 작을 때 발생 가능
			UE_LOG(LogTemp, Error, TEXT("컴포넌트 하나의 단일 틱 당 MovePrediction 최대 처리 횟수에 도달했습니다."));
			break;
		}
	}
}

void UNetPawnInterpComponent::AddNewTransform(FTransform transform, uint32 tick)
{
	if (!bHasBegunMovePrediction) // 최초 transform 추가 시
	{
		pendingTransform = transform;
		bHasBegunMovePrediction = true;
		return;
	}
	AddMovePrediction(pendingTransform, transform, FTimespan::FromSeconds(TimeSinceLastNewTransform), tick);
	TimeSinceLastNewTransform = 0.0f;
	pendingTransform = transform; // 이번 MovePrediction의 종착지는 다음번 MovePrediction의 시작점임
}

void UNetPawnInterpComponent::AddMovePrediction(FTransform from, FTransform to, FTimespan timespan, uint32 tick)
{
	TSharedPtr<MovePrediction> newMovePred = MakeShared<MovePrediction>(from, to, timespan, tick);
	MovePredictionQueue.Enqueue(newMovePred);
	QueueSize++;
	return;
}

float UNetPawnInterpComponent::ApplyMovePrediction(float DeltaTime)
{
	TSharedPtr<MovePrediction> prediction = GetCurrMovePrediction();
	if (!prediction) return 0.0f; // 큐가 비어있음

	TransformAlphaPair pair = prediction->LerpTransform(prediction->GetAlphaFromDeltaTime(DeltaTime));
	if (prediction->Alpha >= 1.0f)
	{
		MovePredictionQueue.Pop(); // 완료 시 큐에서 Pop
		QueueSize--;
	}

	SetPuppetTransform(pair.transform);
	return pair.deltaTimeLeft; // 남은 델타
}

TSharedPtr<MovePrediction> UNetPawnInterpComponent::GetCurrMovePrediction()
{
	if (!MovePredictionQueue.IsEmpty())
		return (*MovePredictionQueue.Peek());
	return nullptr;
}

void UNetPawnInterpComponent::SetPuppetTransform(const FTransform& transform)
{
	ANetSyncPawn* pawn = GetNetPawn();
	if (!pawn || !pawn->IsPuppet())
	{
		UE_LOG(LogTemp, Error, TEXT("NetSyncPawn을 찾을 수 없거나, NetSyncPawn이 Puppet이 아닙니다."));
		return;
	}

	pawn->SetActorTransform(transform);
}

MovePrediction::MovePrediction()
{
}

MovePrediction::MovePrediction(FTransform from, FTransform to, FTimespan timespan, uint32 tick)
{
	this->From = from;
	this->To = to;
	this->Timespan = timespan;
	this->Tick = tick;
}

float MovePrediction::GetAlphaFromDeltaTime(const float& DeltaTime)
{
	return DeltaTime / (float)Timespan.GetTotalSeconds();
}

float MovePrediction::GetDeltaTimeFromAlpha(const float& alpha)
{
	return (float)Timespan.GetTotalSeconds() * alpha;
}

TransformAlphaPair MovePrediction::LerpTransform(const float& DeltaAlpha)
{
	float alphaLeft = 0.0f;
	float setAlphaTo = 0.0f;
	if (IncreaseAlpha(DeltaAlpha) > 1.0f)
	{
		setAlphaTo = 1.0f;
		alphaLeft = Alpha - 1.0f;
		Alpha = setAlphaTo; // 최대 1.0으로 clamp
	}
	else
	{
		setAlphaTo = Alpha;
	}
	return { GameUtilities::LerpTransformNoScale(From, To, setAlphaTo), GetDeltaTimeFromAlpha(alphaLeft) }; // scale을 제외한 트랜스폼 lerp
}

float MovePrediction::IncreaseAlpha(const float& DeltaAlpha)
{
	Alpha += DeltaAlpha;
	return Alpha;
}

