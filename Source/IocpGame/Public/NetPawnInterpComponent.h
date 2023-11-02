// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformTime.h"
#include "Misc/Timespan.h"
#include "NetPawnInterpComponent.generated.h"

class ANetSyncPawn;

struct TransformAlphaPair
{
	FTransform transform;
	float deltaTimeLeft;
};

// Point A to B를 Interpolate 혹은 Extrapolate 하는 것을 나타내는 객체
class MovePrediction
{
public:
	MovePrediction();
	MovePrediction(FTransform from, FTransform to, FTimespan timespan, uint32 tick);

	float GetAlphaFromDeltaTime(const float& DeltaTime); // point a에서 b까지의 timespan 중 DeltaTime이 차지하는 alpha가 얼마인지 반환
	float GetDeltaTimeFromAlpha(const float& alpha); // 역연산
	TransformAlphaPair LerpTransform(const float& DeltaAlpha);
	float IncreaseAlpha(const float& DeltaAlpha);

public:
	FTransform From;
	FTransform To;
	float Alpha = 0.0f;
	FTimespan Timespan;
	uint32 Tick;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IOCPGAME_API UNetPawnInterpComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNetPawnInterpComponent();
	FORCEINLINE ANetSyncPawn* GetNetPawn() { return Cast<ANetSyncPawn>(GetOwner()); }

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void AddNewTransform(FTransform transform, uint32 tick); // 새 최신 transform 정보를 수신받았을 경우 이 함수를 호출해 추가해준다

private:
	float ApplyMovePrediction(float DeltaTime); // 입력받은 Move prediction 객체를 DeltaTime만큼 처리한다. 처리가 완료되었다면 소비하고 남은 DeltaTime을 반환한다.
	TSharedPtr<MovePrediction> GetCurrMovePrediction(); // 현재 시점에 처리해야 할 Move prediction 객체를 반환한다
	void SetPuppetTransform(const FTransform& transform); // 컴포넌트 오너에 transform을 적용한다
	void AddMovePrediction(FTransform from, FTransform to, FTimespan timespan, uint32 tick); // 새 movement interp를 큐에 추가한다

private:
	TQueue<TSharedPtr<MovePrediction>> MovePredictionQueue; // NOTE: Pop이나 Enqueue 시 QueueCount 별도로 관리 필요
	uint32 QueueSize = 0; // 단일 스레드에서만 접근하므로 Atomic 없이 사용 가능
	float DeltaTimeMultiplier = 1.0f; // 큐가 적정치를 초과했을 경우 단일 틱에 여러 MovePrediction을 처리하기 위해 수신받은 델타타임의 크기를 증폭시켜서 여러 MovePrediction에 대한 처리를 가능케 한다
	float TimeSinceLastNewTransform = 0.0f; // 마지막으로 새 트랜스폼을 등록한 이후 경과한 시간 (일반적인 경우 서버로부터 마지막으로 수신받은지 얼마나 시간이 흘렀는지를 의미)
	bool bHasBegunMovePrediction = false; // interpolation을 시작했는지 (지금까지 서버로부터 1개 이상의 transform을 수신받았는지)
	FTransform pendingTransform; // 마지막으로 서버로부터 수신받은 정보
};
