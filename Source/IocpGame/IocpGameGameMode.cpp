// Copyright Epic Games, Inc. All Rights Reserved.

#include "IocpGameGameMode.h"
#include "UObject/ConstructorHelpers.h"

AIocpGameGameMode::AIocpGameGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_HostPlayerPawn"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("게임모드 초기 폰 클래스를 찾을 수 없습니다. 파일 경로를 확인해 주세요."));
	}
}
