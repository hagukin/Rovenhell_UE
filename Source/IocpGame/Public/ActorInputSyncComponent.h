// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RovenhellGameInstance.h"
#include "GameTickCounter.h"
#include "InputActionValue.h"
#include "ActorInputSyncComponent.generated.h"

#define MAX_INPUTS_HISTORY_SIZE 60 // 틱 당 허용되는 최대 인풋수 (20) * 서버의 State 전송 rate와 클라 틱 rate의 ratio (125 vs 100 = 1.25) 이상; Min 2
#define INPUT_TIME_ADJUSTMENT 0.125f // (s) 클라 시간이 서버 시간보다 최소 이만큼 앞서있도록 보정해 인풋 재처리가 강제적으로 일어나게 함으로써 보정 시 끊김을 완화한다

struct LocalInputs
{
	uint32 ActionType = ActionTypeEnum::UNDEFINED;
	int AxisDimension = 0;
	FVector Value = { 0,0,0 };
	float DeltaTime = 0.0f; // 재연산을 위해 필요한 값
	float InputTime = 0.0f; // 발생 시점
	FVector PlayerFacingDirection; // 인풋 발생 시점에서의 플레이어 지향 방향; 추후 재연산을 위해 사용
};

/**
 * 인풋을 입력 가능한 Pawn에 대해 싱크를 맞춘다.
 * e.g. 클라이언트에서 '내 플레이어 캐릭터'는 내 인풋에 의해 조종되기 때문에 해당 컴포넌트를 사용한다.
 * 반대로 '다른 플레이어 캐릭터'는 내가 직접 관여할 수 없기 때문에 ActorSyncComponent를 사용한다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class IOCPGAME_API UActorInputSyncComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UActorInputSyncComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void AdjustInputHistoryCursor() { if (bShouldMoveCursor) { MoveOneInputsHistoryCursor(); bShouldMoveCursor = false; } } // InputHistory 커서를 움직여야 할 경우 움직인다
protected:
	virtual void BeginPlay() override;
	void MoveOneInputsHistoryCursor() { InputHead = (InputHead + 1) % MAX_INPUTS_HISTORY_SIZE; InputTail = (InputTail + 1) % MAX_INPUTS_HISTORY_SIZE; } // 헤드 및 테일을 1씩 이동시킨다

public:
	void AddInputsInfo(ActionTypeEnum actionType, const FInputActionValue& Value, float deltaTime, float time, FVector PlayerFacingDirection); // Tail에 덮어쓰고 커서를 한 칸씩 이동시킨다
	void ReapplyLocalInput(const LocalInputs& input);
	void ReapplyLocalInputAfter(float time); // time을 초과하는 인풋들부터 재처리해 현재 상태에 반영한다.
	// 서버 수신 정보는 과거의 정보이기 때문에, 해당 정보 반영 후 이미 호스트에서 입력한 인풋들에 대한 처리가 다시 한 번 이루어져야 한다.
	// 서버 수신 정보로부터 각 세션들의 로컬 틱 N번까지 처리가 완료되었다는 정보를 추출해, 그 해당 틱 이후의 인풋들을 재처리해야 한다.

protected:
	uint32 InputHead = 0; // 가장 오래된 정보
	uint32 InputTail = MAX_INPUTS_HISTORY_SIZE - 1; // 가장 뒷번 인덱스로 초기화; 가장 최근 정보
	TStaticArray<LocalInputs, MAX_INPUTS_HISTORY_SIZE> InputsHistory; // Circular Array로 사용; SD_GameInputHistory와 다르게 단위 시간 동안만 기록하는 게 아니라 지속적으로 인풋을 기록한다

private:
	bool bShouldMoveCursor = false; // 이번 틱에 AddInputsInfo가 호출되어 커서를 다음 칸으로 이동해야 하는지; true일 경우 틱이 종료되는 시점에 
};
