// Copyright Epic Games, Inc. All Rights Reserved.

#include "IocpGameGameMode.h"
#include "IocpGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AIocpGameGameMode::AIocpGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
