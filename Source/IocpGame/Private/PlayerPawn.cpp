// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"

APlayerPawn::APlayerPawn()
{
	// 3인칭 카메라 - 마우스 회전은 캐릭터를 회전시키지 않는다
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캡슐 컴포넌트
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp); // 물리적 실체가 있는 것으로 루트 교환
	CapsuleComp->SetSimulatePhysics(true);

	// 메쉬
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);

	// 카메라용 SpringArm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 카메라 거리
	CameraBoom->bUsePawnControlRotation = true; // 폰과 함께 회전

	// 카메라
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 스프링암 끝 소켓에 고정
	FollowCamera->bUsePawnControlRotation = false; // 회전X, 스프링암에 고정

	// 무브먼트
	MovementComp = CreateDefaultSubobject<UNetPlayerMovementComponent>(TEXT("MovementComp"));
	MovementComp->UpdatedComponent = RootComponent; // 씬 컴포넌트 아니라 어태치하지 않아도 됨

	// 인풋 펜딩 어레이 생성
	GameInputPendings = MakeShared<TArray<SD_GameInput>>();
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerPawn::Move_Entry);

		// 시야 회전
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerPawn::Look);
	}
}

void APlayerPawn::BeginPlay()
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
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerPawn::Move(const FInputActionValue& Value, float DeltaTime)
{
	// 실제 이동은 MovementComponent에서 처리한다.
	// 여기서는 이동 요청을 처리한다.
	FVector2D MovementVector = Value.Get<FVector2D>();

	AController* controller = GetController();
	if (!controller)
	{
		UE_LOG(LogTemp, Error, TEXT("컨트롤러가 없어 폰에 대한 인풋을 처리할 수 없습니다!"));
		return;
	}

	// 전면 방향 계산
	const FRotator rotation = controller->GetControlRotation();
	const FRotator yawRotation(0, rotation.Yaw, 0);

	const FVector forwardDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	MovementComp->AddMovementData((forwardDirection * MovementVector.Y) + (rightDirection * MovementVector.X), DeltaTime);
}

void APlayerPawn::Move_Entry(const FInputActionValue& Value)
{
	ANetHandler* netHandler = GetNetHandler();
	if (!netHandler)
	{
		UE_LOG(LogTemp, Fatal, TEXT("NetHandler를 찾을 수 없습니다."));
		return;
	}

	switch (netHandler->GetHostType())
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
		{
			Move_UEClient(Value, Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetDelta()); // 클라이언트 사이드 연산에서는 현재 호스트 델타 사용
			break;
		}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
		{
			Move_UEServer(Value, Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetDelta()); // 디버깅 목적으로 서버에서 키 인풋으로 직접 움직이는 경우는 서버 호스트 델타 사용
			break;
		}
		case HostTypeEnum::NONE:
		default:
			break;
	}
}

void APlayerPawn::Move_UEClient(const FInputActionValue& Value, float DeltaTime)
{
	Move(Value, DeltaTime);





	//////////// TESTING
	GameInputPendings->Add(SD_GameInput(ActionTypeEnum::MOVE, Value, DeltaTime));
}

void APlayerPawn::Move_UEServer(const FInputActionValue& Value, float DeltaTime)
{
	Move(Value, DeltaTime);
	return;
}

void APlayerPawn::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X); // TODO: 서버클라 구현
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
