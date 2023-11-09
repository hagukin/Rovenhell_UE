// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "PlayerPawn.h"

APlayerPawn::APlayerPawn()
{
	// 인풋 싱크 컴포넌트
	InputSyncComp = CreateDefaultSubobject<UActorInputSyncComponent>(TEXT("InputSyncComp"));
	this->AddOwnedComponent(InputSyncComp);

	// 3인칭 카메라 - 마우스 회전은 캐릭터를 회전시키지 않는다
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캡슐 컴포넌트
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp); // 물리적 실체가 있는 것으로 루트 교환
	CapsuleComp->SetSimulatePhysics(true);
	CapsuleComp->SetPhysicsMaxAngularVelocityInDegrees(0); // 의도되지 않은 회전 제한

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
	}
}

void APlayerPawn::Puppetfy()
{
	Super::Puppetfy();

	// 퍼펫에게 불필요한 컴포넌트들 삭제
	FollowCamera->UnregisterComponent();
	FollowCamera->DestroyComponent();

	CameraBoom->UnregisterComponent();
	CameraBoom->DestroyComponent();

	InputSyncComp->UnregisterComponent();
	InputSyncComp->DestroyComponent();
	GameInputPendings = nullptr;

	// NOTE: MovementComp의 경우 서버 퍼펫은 있어야 하지만 클라 퍼펫은 없어야 함.
	// 이는 서버는 인풋을 재연산해서 움직임을 처리하고, 클라는 연산 결과를 interp해서 처리하기 때문.
	// 만약 클라 퍼펫이 MovementComp를 가지게 되는 경우 MovementComp 및 GameStateApplier 양쪽에서 모두 애니메이션에 접근해 State를 수정하게 되므로
	// 애니메이션이 정상적으로 처리되지 않을 수 있음.
	HostTypeEnum hostType = GetNetHandler()->GetHostType();
	if (hostType == HostTypeEnum::CLIENT || hostType == HostTypeEnum::CLIENT_HEADLESS)
	{
		MovementComp->UnregisterComponent();
		MovementComp->DestroyComponent();
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
	MovementComp->AddMovementData(FVector(MovementVector.Y, MovementVector.X, 0), DeltaTime); // 처리 큐에 추가
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
			Move_UEClient(Value, Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetDelta()); // Host deltatime 사용
			break;
		}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
		{
			Move_UEServer(Value, Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetDelta()); // 서버에서 디버깅 목적으로 호스트 폰을 움직이는 경우 Host deltatime 사용
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
	float time = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetTime(GetWorld());
	float adjustedTime = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetAdjustedTime_UEClient(GetWorld(), INPUT_TIME_ADJUSTMENT);
	GameInputPendings->Add(SD_GameInput(ActionTypeEnum::MOVE, Value, DeltaTime, time));
	GetInputSyncComp()->AddInputsInfo(ActionTypeEnum::MOVE, Value, DeltaTime, adjustedTime/*인풋 보정을 위해 서버시간보다 앞서있는 시간을 전달*/, GetMovementComponent()->GetCurrentFacingDirection()); // 커서 이동은 나중에 해준다
}

void APlayerPawn::Move_UEServer(const FInputActionValue& Value, float DeltaTime)
{
	Move(Value, DeltaTime);
	return;
}

void APlayerPawn::SetAllAnimBranchToFalse()
{
	bIsIdling = false; 
	bIsMoving = false;
}

void APlayerPawn::SetAnimBranchToIdling()
{
	SetAllAnimBranchToFalse();
	bIsIdling = true;
}

void APlayerPawn::SetAnimBranchToMoving()
{
	SetAllAnimBranchToFalse();
	bIsMoving = true;
}

void APlayerPawn::SetAnimTo(AnimStateEnum state, float value1D)
{
	SetAnimStateTo(state); // state를 먼저 세팅해야 status가 올바르게 적용된다
	SetAnimStatusTo(value1D);
}

void APlayerPawn::SetAnimStateTo(AnimStateEnum state)
{
	if (state == CurrAnimState) return; // 동일

	// FSM 브랜치
 	switch (state)
	{
		case AnimStateEnum::MOVING:
		{
			SetAnimBranchToMoving();
			break;
		}
		case AnimStateEnum::IDLING:
		{
			SetAnimBranchToIdling();
			break;
		}
		default:
			break;
	}

	CurrAnimState = state;
}

void APlayerPawn::SetAnimStatusTo(float value1D)
{
	switch (CurrAnimState)
	{
		case AnimStateEnum::MOVING:
		{
			MovementStatus = value1D;
			return;
		}
	}
}

float APlayerPawn::GetCurrentAnimStatus()
{
	switch (CurrAnimState)
	{
		case AnimStateEnum::MOVING:
		{
			return MovementStatus;
		}
		default:
			return 0.0f; // blendspace를 사용하지 않는 경우 0.0 반환
	}
}

void APlayerPawn::AddMovementStatus(float value)
{
	MovementStatus = FMath::Clamp(MovementStatus + value, 0.0f, MaxMovementStatus);
}
