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

	// 물리 연산
	// 주의: PawnMovement에 기본적으로 구현된 함수들(AddInputVector 등)은 우리의 넷코드와 호환되지 않기 때문에 사용하지 않는다
	void AddMovementData(FVector MoveVector, float DeltaTime); // 콜스택: InputApplier -> Move

private:
	APlayerPawn* GetPlayer();
	FVector GetCurrentFacingDirection();

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

	// 이동 애니메이션
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float MovementStatus = 0.0f; // 0 초과일 경우 해당 캐릭터가 이동중임을 나타낸다. 한 번의 이동 입력 시 일정한 값만큼 증가한다
	float MaxMovementStatus = 100.0f;
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
