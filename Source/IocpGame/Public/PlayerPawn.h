// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "NetSyncPawn.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "../Enumerations.h"
#include "NetPlayerMovementComponent.h"
#include "ActorInputSyncComponent.h"
#include "Animation/AnimNode_StateMachine.h"
#include "PlayerPawn.generated.h"

/**
 * 
 */
UCLASS()
class IOCPGAME_API APlayerPawn : public ANetSyncPawn
{
	GENERATED_BODY()
	
public:
	APlayerPawn();

	// 컨트롤러 bind
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Puppetfy() override;

	// Getter
	UActorInputSyncComponent* GetInputSyncComp() const { return InputSyncComp; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE virtual UNetPlayerMovementComponent* GetMovementComponent() const { return MovementComp; };

	// 액션
	void Move(const FInputActionValue& Value, float DeltaTime);
	void Move_Entry(const FInputActionValue& Value); // 키 입력에 대한 처리
	void Move_UEClient(const FInputActionValue& Value, float DeltaTime);
	void Move_UEServer(const FInputActionValue& Value, float DeltaTime);

	TSharedPtr<TArray<SD_GameInput>> GetGameInputPendings() { return GameInputPendings; }
	void ClearGameInputPendings() { GameInputPendings->Empty(); }

	// 애니메이션
	void SetAllAnimBranchToFalse();
	void SetAnimBranchToIdling();
	void SetAnimBranchToMoving();
	void SetAnimTo(AnimStateEnum state) { return SetAnimStateTo(state); } // value 수정을 하지 않는다
	void SetAnimTo(AnimStateEnum state, float value1D);
	AnimStateEnum GetCurrentAnimState() { return CurrAnimState; }
	float GetCurrentAnimStatus(); // 현재 AnimState에서 사용중인 blendspace 값을 반환한다
	float GetSavedAnimStatus(); // 마지막으로 save한 AnimState blendspace 값을 반환한다, 없을 경우 0을 반환한다
	const float GetMaxAnimStatus() { return MaxMovementStatus; }
	void SaveCurrentAnimStatus(); // 현재 AnimState blendspace 값을 save한다
	void AddMovementStatus(float value); // 음수 더해 뺄셈도 가능

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

	// 애니메이션
	void SetAnimStateTo(AnimStateEnum state, bool isForced = false); // 퍼펫의 위치 interp 중이더라도 애니메이션을 강제로 변경하기 위해 isForced를 true로 설정할 수 있다
	void ApplyAnimStatusDelta(float value1D); // NOTE: 추후 2D blendspace 사용 시 추가

public:
	//// 컴포넌트
	// 싱크 컴포넌트
	UActorInputSyncComponent* InputSyncComp;

	// 콜리전, 피직스, Root
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* CapsuleComp;

	// 렌더링
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* SkeletalMeshComp;

	// 카메라 스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// 무브먼트
	UNetPlayerMovementComponent* MovementComp;

	//// 인풋 액션
	// 입력과 인풋 액션 맵핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext; // BP

	// 이동
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction = nullptr; // BP

	// 시야
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction = nullptr; // BP

private:
	//// 애니메이션
	AnimStateEnum CurrAnimState = AnimStateEnum::NO_ANIM;

	// blendspace 값들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float MovementStatus = 0.0f; // 0 초과일 경우 해당 캐릭터가 이동중임을 나타낸다. 이동 애니메이션 blendspace에 사용됨
	float MaxMovementStatus = 100.0f;
	float SavedMovementStatus = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bIsIdling = false;

private:
	/* UEClient */
	TSharedPtr<TArray<SD_GameInput>> GameInputPendings; // 서버로 발송 대기중인 클라이언트이 게임플레이 인풋들
};
