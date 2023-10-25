// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Templates/SharedPointer.h"
#include "../ExecType.h"
#include "GameTickCounter.h"
#include "NetActorSpawner.h"
#include "RovenhellGameInstance.generated.h"

class ANetHandler;
class APlayerPawn;

/**
 * 
 */
UCLASS()
class IOCPGAME_API URovenhellGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	URovenhellGameInstance();

	const TSharedPtr<ExecType> GetExecType() { return GameExecType; }

	// TODO: 런타임 이전에 호스트 타입을 정할 수 있도록 외부 장치 추가
	// 현재는 편의상 런타임에 유저가 결정 가능
	// 단 현재 방식으로는 Reflection이 Enum을 감지하지 못하기 때문에 수동으로 값을 입력해야 함
	UFUNCTION(BlueprintCallable)
	void SetHostTypeAs(uint8 hostType) { GameExecType->SetHostType((HostTypeEnum)hostType); }

	//// 현재 접속한 세션들의 플레이어 컨트롤러 관리
	bool AddPlayer(uint16 sessionId, APlayerPawn* pawn);
	bool RemovePlayer(uint16 sessionId); // 실제 폰 객체에 대한 삭제 처리는 별도로 해주어야 함
	APlayerPawn* GetPlayerOfOwner(uint16 sessionId); // 반환값은 다음 사이클(혹은 그 이후)에 GC에 의해 삭제될 수 있기 때문에 함수를 호출한 해당 사이클에서만 반환값의 유효성이 보장됨. 따라서 반환 값을 다른 곳에 캐싱하는 것은 권장되지 않음.
	const TMap<uint16, TWeakObjectPtr<APlayerPawn>>& GetPlayers() { return Players; }

	TWeakObjectPtr<ANetHandler> GetNetHandler(); // 현재 월드에 있는 NetHandler를 반환한다. 없을 경우 nullptr를 반환한다.
	TWeakObjectPtr<ANetActorSpawner> GetNetActorSpawner(); // 현재 월드에 있는 NetActorSpanwer를 반환한다. 없을 경우 nullptr를 반환한다.

public:
	TUniquePtr<GameTickCounter> TickCounter = nullptr;
	mutable FCriticalSection PlayersInfoLock;

private:
	TSharedPtr<ExecType> GameExecType = nullptr;
	TMap<uint16, TWeakObjectPtr<APlayerPawn>> Players; // 외부에서의 해제 시 처리를 위해 TWeakObjectPtr 사용 TODO: 테스트 필요
	TWeakObjectPtr<ANetHandler> NetHandlerWeakPtr = nullptr; // 성능 향상을 위해 캐싱
	TWeakObjectPtr<ANetActorSpawner> NetActorSpawnerWeakPtr = nullptr; // 성능 향상을 위해 캐싱
};
