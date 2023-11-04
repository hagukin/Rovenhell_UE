// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "NetPawnMovementComponent.h"
#include "NetHandler.h"
#include "NetPlayerMovementComponent.generated.h"

// 단일 인풋에 대한 데이터
typedef struct MoveInputData
{
	FVector MoveVector;
	float DeltaTime;
};

class APlayerPawn;
/**
 * 
 */
UCLASS()
class IOCPGAME_API UNetPlayerMovementComponent : public UNetPawnMovementComponent
{
	GENERATED_BODY()
	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	APlayerPawn* GetPlayerOwner();
	ANetHandler* GetNetHandler();

	void BeginTick();
	void EndTick(float HostDeltaTime);

	// Getter
	FVector GetCurrentFacingDirection();
	APlayerPawn* GetPlayer();

	// 물리 연산
	// 주의: PawnMovement에 기본적으로 구현된 함수들(AddInputVector 등)은 우리의 넷코드와 호환되지 않기 때문에 사용하지 않는다
	void AddMovementData(FVector MoveVector, float DeltaTime); // 콜스택: InputApplier -> Move
	void ApplySingleMoveInputData(const MoveInputData& moveInputData);
	void ApplySingleMoveInputData(const MoveInputData& moveInputData, FVector CustomPlayerFacingDirection); // 인풋 재연산 시 과거 인풋 시점의 FacingDirection을 건내줄 때 사용

	FORCEINLINE bool CanFallOffLedge() { return bCanFallOffLedge; }
private:
	void ApplyMovement(FVector FacingDirection, float InputDeltaTime);
	void ApplyRotation(FVector FacingDirection, float InputDeltaTime);

private:
	// 이번 틱에 처리해야 하는 Move 인풋(들)의 델타 타임
	// 로직 서버의 경우 단일 폰에 대한 인풋 여러 번을 한번에 처리해야 하므로 Array의 크기가 1 이상일 수 있다.
	// 이 경우 어레이의 크기를 이용해 이번 틱에 처리되야 하는 총 Move 인풋의 개수를 구하기 때문에
	// TickComponent가 처리되는 동안에는 개수를 늘이거나 줄여서는 안된다
	TArray<MoveInputData> MoveDatas;

	//// 이동
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	float PlayerSpeed = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool bCanFallOffLedge = false; // bCanWalkOffLedge가 절벽에서 떨어지는 것을 방지한다면, 이 값은 모든 종류의 falling을 방지해 중력 연산을 사전에 막는다
	const float MoveEstimationDistance = 0.01f; // 이동방향에서 fall 판정을 하기 위해 얼마만큼 이동할 것인가
	const float MinFallOffDistance = 0.1f; // 이 거리보다 높은 고저차는 fall로 간주
	const float FallingTraceDistance = 1e5; // 이 거리만큼 fall 트레이스

	// 이동 애니메이션
	// PlayerPawn 내의 MovementStatus값을 인풋에 따라 변경한다
	float MoveStatIncrementPerInput = 25.0f; // 인풋 한 번당 증가하는 양
	float MoveStatDecrementPerTick = 25.0f; // 인풋이 없었을 경우 1틱 당 감소하는 양

	// 애니메이션 싱크
	int MoveInputCountThisTick = 0; // 이번 틱 동안 처리한 인풋들의 개수; 서버에서 클라와 애니메이션 Blendspace를 싱킹하기 위해 사용한다.
	float MoveInputDeltaTimeSumThisTick = 0.0f; // 이번 틱 동안 처리한 인풋들의 델타타임의 합; 서버가 framerate와 무관하게 애니메이션을 클라와 동일하게 처리하기 위해 사용한다

	//// 회전
	FVector DesiredDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	float TurnLeftYaw = -650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	float TurnRightYaw = 650.0f;
};
