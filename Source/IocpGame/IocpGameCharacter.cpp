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
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// 3인칭 카메라 - 마우스 회전은 캐릭터를 회전시키지 않는다
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // 인풋 방향으로 이동
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 회전 rate

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// 카메라용 SpringArm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 카메라 거리
	CameraBoom->bUsePawnControlRotation = true; // 폰과 함께 회전

	// 카메라
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 스프링암 끝 소켓에 고정
	FollowCamera->bUsePawnControlRotation = false; // 회전X, 스프링암에 고정
}

void AIocpGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 키 입력과 인풋 액션 맵핑
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
		UE_LOG(LogTemp, Warning, TEXT("ANetHandler 클래스의 액터를 찾을 수 없습니다."));
	}
	else
	{
		NetHandler = Cast<ANetHandler>(temp);
	}
}

void AIocpGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// 점프
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AIocpGameCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AIocpGameCharacter::JumpStart); // 누른 최초 1회
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AIocpGameCharacter::StopJumping);

		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIocpGameCharacter::Move_Entry);

		// 시야 회전
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIocpGameCharacter::Look);
	}
}

void AIocpGameCharacter::Move(const FInputActionValue& Value, float DeltaRatio)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Log, TEXT("틱 %i Move"), Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetTick());

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
		AddMovementInput(ForwardDirection, MovementVector.Y * DeltaRatio); // DeltaRatio - 클라: 1.0f; 서버: 클라 델타 / 서버 델타
		AddMovementInput(RightDirection, MovementVector.X * DeltaRatio);
	}
}

void AIocpGameCharacter::Move_Entry(const FInputActionValue& Value)
{
	switch (NetHandler->GetHostType())
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
			{
				Move_UEClient(Value);
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				Move_UEServer(Value, 1.0f); // 디버깅 목적으로 서버에서 키 인풋으로 직접 움직이는 경우는 DeltaRatio 1로 설정
				break;
			}
		case HostTypeEnum::NONE:
		default:
			break;
	}
}

void AIocpGameCharacter::Move_UEClient(const FInputActionValue& Value)
{
	Move(Value, 1.0f);

	// TESTING
	TSharedPtr<SendBuffer> writeBuf;
	while (!writeBuf) writeBuf = NetHandler->GetSessionShared()->BufManager->SendPool->PopBuffer();
	NetHandler->GetSerializerShared()->Clear();
	SD_GameInput* inputData = new SD_GameInput(ActionTypeEnum::MOVE, Value);
	NetHandler->GetSerializerShared()->Serialize((SD_Data*)inputData);
	NetHandler->GetSerializerShared()->WriteDataToBuffer(writeBuf);
	NetHandler->FillPacketSenderTypeHeader(writeBuf);
	((PacketHeader*)(writeBuf->GetBuf()))->senderId = NetHandler->GetSessionShared()->GetSessionId();
	((PacketHeader*)(writeBuf->GetBuf()))->protocol = PacketProtocol::CLIENT_EVENT_ON_RECV;
	((PacketHeader*)(writeBuf->GetBuf()))->id = PacketId::GAME_INPUT;
	((PacketHeader*)(writeBuf->GetBuf()))->tick = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetTick();
	((PacketHeader*)(writeBuf->GetBuf()))->tick = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetDelta();
	NetHandler->GetSessionShared()->PushSendQueue(writeBuf);
}

void AIocpGameCharacter::Move_UEServer(const FInputActionValue& Value, float DeltaRatio)
{
	Move(Value, DeltaRatio);
	return;
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
	
}

void AIocpGameCharacter::Jump_UEServer()
{
	// TESTING
	TSharedPtr<SendBuffer> writeBuf;
	while (!writeBuf) writeBuf = NetHandler->GetSessionShared()->BufManager->SendPool->PopBuffer();
	NetHandler->GetSerializerShared()->Clear();
	SD_Transform* transformData = new SD_Transform(&this->GetTransform());
	NetHandler->GetSerializerShared()->Serialize((SD_Data*)transformData);
	NetHandler->GetSerializerShared()->WriteDataToBuffer(writeBuf);
	NetHandler->FillPacketSenderTypeHeader(writeBuf);
	((PacketHeader*)(writeBuf->GetBuf()))->senderId = NetHandler->GetSessionShared()->GetSessionId();
	((PacketHeader*)(writeBuf->GetBuf()))->protocol = PacketProtocol::LOGIC_EVENT;
	((PacketHeader*)(writeBuf->GetBuf()))->id = PacketId::ACTOR_PHYSICS;
	((PacketHeader*)(writeBuf->GetBuf()))->tick = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetTick();
	((PacketHeader*)(writeBuf->GetBuf()))->deltaTime = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetDelta();
	NetHandler->GetSessionShared()->PushSendQueue(writeBuf);
}
