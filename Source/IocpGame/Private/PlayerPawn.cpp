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
	uint32 tick = Cast<URovenhellGameInstance>(GetGameInstance())->TickCounter->GetLocalTick(); // 주의: 반드시 로컬 틱을 보내기 및 히스토리에 저장해야함
	GameInputPendings->Add(SD_GameInput(ActionTypeEnum::MOVE, Value, DeltaTime, tick));
	GetInputSyncComp()->AddInputsInfoAndMoveOne(ActionTypeEnum::MOVE, Value, DeltaTime, tick);
}

void APlayerPawn::Move_UEServer(const FInputActionValue& Value, float DeltaTime)
{
	Move(Value, DeltaTime);
	return;
}

void APlayerPawn::SetAllAnimStateToFalse()
{
	bIsIdling = false; 
	bIsMoving = false;
}

void APlayerPawn::SetIsIdlingTo(bool value)
{
	if (bIsIdling == value) return;
	SetAllAnimStateToFalse(); // TODO FIXME
	bIsIdling = value;
}

void APlayerPawn::SetIsMovingTo(bool value)
{
	if (bIsMoving == value) return;
	SetAllAnimStateToFalse(); // TODO FIXME
	bIsMoving = value;
}
