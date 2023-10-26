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

void UActorInputSyncComponent::ReapplyLocalInput(const LocalInputs& input)
{
	if (APlayerPawn* player = Cast<APlayerPawn>(GetOwner()))
	{
		if (input.ActionType == ActionTypeEnum::MOVE)
		{
			FVector prev = player->GetActorLocation();
			player->SetActorRotation(input.PlayerFacingDirection.Rotation()); // 우선 인풋 시점의 방향으로 회전시킨다
			Cast<UNetPlayerMovementComponent>(player->GetMovementComponent())->ApplySingleMoveInputData({ input.Value, input.DeltaTime }, input.PlayerFacingDirection); // 인풋 시점의 플레이어 방향 전달

			DrawDebugLine(GetWorld(), prev, player->GetActorLocation(), FColor(255, 255, 0), true);
			DrawDebugSphere(GetWorld(), player->GetActorLocation(), 5, 1, FColor(255, 255, 0), true);
		}
	}
	return;
}

void UActorInputSyncComponent::ReapplyLocalInputAfter(uint32 tick)
{
	if (tick >= InputsHistory[InputTail].InputTick) // >=
	{
		return; // 클라이언트 framerate가 서버보다 낮고 네트워크 전송시간이 아주 빠를 경우 발생할 수 있다
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
		ReapplyLocalInput(InputsHistory[i]);
	}
}

void UActorInputSyncComponent::AddInputsInfo(ActionTypeEnum actionType, const FInputActionValue& Value, float deltaTime, uint32 tick, FVector PlayerFacingDirection)
{
	LocalInputs input = { actionType, (int)Value.GetValueType(), Value.Get<FVector>(), deltaTime, tick, PlayerFacingDirection };
	InputsHistory[InputTail] = input;
	bShouldMoveCursor = true;
}
