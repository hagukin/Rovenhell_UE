// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetSyncPawn.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "NetPlayerMovementComponent.h"
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

	// Getter
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComp; };

	// 액션
	void Move(const FInputActionValue& Value, float DeltaTime);
	void Move_Entry(const FInputActionValue& Value); // 키 입력에 대한 처리
	void Move_UEClient(const FInputActionValue& Value, float DeltaTime);
	void Move_UEServer(const FInputActionValue& Value, float DeltaTime);
	void Look(const FInputActionValue& Value);

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

public:
	//// 컴포넌트
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
};
