// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "IocpGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AIocpGameCharacter::AIocpGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AIocpGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// TEMP
	// NetHandler 레퍼런스 가져오기
	AActor* temp =  UGameplayStatics::GetActorOfClass(GetWorld(), ANetHandler::StaticClass());
	if (!temp)
	{
		UE_LOG(LogTemp, Error, TEXT("ANetHandler 클래스의 액터를 찾을 수 없습니다."));
	}
	else
	{
		NetHandler = Cast<ANetHandler>(temp);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AIocpGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AIocpGameCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AIocpGameCharacter::JumpStart); // 누른 최초 1회
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AIocpGameCharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIocpGameCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIocpGameCharacter::Look);

	}

}

void AIocpGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AIocpGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AIocpGameCharacter::JumpStart() // 테스트용 함수; OnTrigger와 다르게 시작시점 딱 한 틱에서 실행
{
	switch (NetHandler->GetHostType())
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
			{
				Jump_UEClient();
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				Jump_UEServer();
				break;
			}
		case HostTypeEnum::NONE:
		default:
			break;
	}
}

void AIocpGameCharacter::Jump_UEClient()
{
	// TESTING
	T_BYTE testJumpPacket[4] = { T_BYTE(74), T_BYTE(85), T_BYTE(77), T_BYTE(80) }; //JUMP
	TSharedPtr<SendBuffer> writeBuf;
	while (!writeBuf) writeBuf = NetHandler->GetSessionShared()->BufManager->SendPool->PopBuffer();
	writeBuf->Write(testJumpPacket, sizeof(testJumpPacket));
	NetHandler->FillPacketSenderTypeHeader(writeBuf);
	((PacketHeader*)(writeBuf->GetBuf()))->senderId = NetHandler->GetSessionShared()->GetSessionId();
	((PacketHeader*)(writeBuf->GetBuf()))->protocol = PacketProtocol::CLIENT_EVENT_ON_RECV;
	((PacketHeader*)(writeBuf->GetBuf()))->id = PacketId::ACTOR_PHYSICS;
	((PacketHeader*)(writeBuf->GetBuf()))->tick = 99999;
	NetHandler->GetSessionShared()->PushSendQueue(writeBuf);
}

void AIocpGameCharacter::Jump_UEServer()
{

}
