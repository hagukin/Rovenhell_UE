// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "ActorInputSyncComponent.h"
#include "PlayerPawn.h"

UActorInputSyncComponent::UActorInputSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick에서 로직을 처리하는 컴포넌트가 아님;
}

void UActorInputSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	HostTypeEnum hostType = Cast<URovenhellGameInstance>(GetWorld()->GetGameInstance())->GetExecType()->GetHostType();
	if (hostType == HostTypeEnum::CLIENT || hostType == HostTypeEnum::CLIENT_HEADLESS)
	{
		for (int i = 0; i < MAX_INPUTS_HISTORY_SIZE; ++i)
		{
			InputsHistory[i] = { ActionTypeEnum::UNDEFINED, 0, FVector(0,0,0), 0.0f, 0 }; // 사용 불가능한 인풋으로 초기화
		}
	}
}

void UActorInputSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	return;
}

void UActorInputSyncComponent::ApplyInput(const LocalInputs& input)
{
	if (APlayerPawn* player = Cast<APlayerPawn>(GetOwner()))
	{
		if (input.ActionType == ActionTypeEnum::MOVE)
		{
			player->Move(input.Value, input.DeltaTime);
		}
	}
	return;
}

void UActorInputSyncComponent::ReapplyLocalInputAfter(uint32 tick)
{
	// [서버에서 수신받은 시점의 틱, 기록된 마지막 인풋 틱] 범위에 해당하는 인풋들을 재처리한다
	if (tick >= InputsHistory[InputTail].InputTick)
	{
		// 서버 틱이 클라이언트 인풋 히스토리의 가장 마지막 틱보다 같거나 클 경우 인풋 재처리를 진행하지 않는다
		return;
	}

	int applyStartIndex = InputHead;
	for (int addIdx = 0; addIdx < MAX_INPUTS_HISTORY_SIZE; ++addIdx)
	{
		if (InputsHistory[(InputHead + addIdx) % MAX_INPUTS_HISTORY_SIZE].InputTick > tick)
		{
			applyStartIndex = (InputHead + addIdx) % MAX_INPUTS_HISTORY_SIZE;
			break;
		}
	}

	// applyStartIndex 인덱스부터 Tail까지를 순회하며 인풋들을 적용한다
	for (int i = applyStartIndex; i != InputTail; i = (i + 1) % MAX_INPUTS_HISTORY_SIZE)
	{
		ApplyInput(InputsHistory[i]);
	}
}

void UActorInputSyncComponent::AddInputsInfoAndMoveOne(ActionTypeEnum actionType, const FInputActionValue& Value, float deltaTime, uint32 tick)
{
	LocalInputs input = { actionType, (int)Value.GetValueType(), Value.Get<FVector>(), deltaTime, tick };
	InputsHistory[InputTail] = input;
	MoveOneInputsHistoryCursor();
}
