// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NetHandler.h"
#include "NetCodes/NetBuffer.h"
#include "IocpGameCharacter.generated.h"


UCLASS(config=Game)
class AIocpGameCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AIocpGameCharacter();

	void Move(const FInputActionValue& Value);
	void Move_Entry(const FInputActionValue& Value);
	void Move_UEClient(const FInputActionValue& Value);
	void Move_UEServer(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	// TODO

	void JumpStart();
	void Jump_UEClient();
	void Jump_UEServer();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay();

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	//// 컴포넌트
	// 카메라 스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


	//// 인풋 액션
	// 입력과 인풋 액션 맵핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext; // BP

	// 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction = nullptr; // BP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction = nullptr; // BP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction = nullptr; // BP

	// SkeletalMesh, AnimBP는 블루프린트에서 초기화
private:
	ANetHandler* NetHandler = nullptr;
};

