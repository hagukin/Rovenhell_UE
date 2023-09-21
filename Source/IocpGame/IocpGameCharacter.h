// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NetHandler.h"
#include "NetCodes/NetBuffer.h"
#include "NetSyncPawn.h"
#include "ActorPhysicsSyncComponent.h"
#include "IocpGameCharacter.generated.h"


UCLASS(config=Game)
class AIocpGameCharacter : public ANetSyncPawn
{
	GENERATED_BODY()

};

