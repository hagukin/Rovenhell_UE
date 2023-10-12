// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "RovenhellGameInstance.h"
#include "NetHandler.h"
#include "PlayerPawn.h"

URovenhellGameInstance::URovenhellGameInstance()
{
	GameExecType = MakeShared<ExecType>();
	TickCounter = MakeUnique<GameTickCounter>();
}

bool URovenhellGameInstance::AddPlayer(uint64 sessionId, APlayerPawn* player)
{
    if (!Players.Contains(sessionId))
    {
        while (!PlayersInfoLock.TryLock());
        Players.Add(sessionId, TWeakObjectPtr<APlayerPawn>(player));
        PlayersInfoLock.Unlock();
    }
    else
    {
        while (!PlayersInfoLock.TryLock());
        UE_LOG(LogTemp, Warning, TEXT("세션 아이디 %i번 플레이어에 대한 정보가 이미 존재합니다. 값을 덮어 씁니다."), sessionId);
        Players[sessionId] = player;
        PlayersInfoLock.Unlock();
    }
    return true;
}

bool URovenhellGameInstance::RemovePlayer(uint64 sessionId)
{
    if (Players.Contains(sessionId))
    {
        while (!PlayersInfoLock.TryLock());
        Players.Remove(sessionId); // 실제 객체 해제는 별도로 처리해주어야 한다
        PlayersInfoLock.Unlock();
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 아이디 %i번 플레이어에 대한 정보가 존재하지 않아 삭제할 수 없습니다."), sessionId);
        return false;
    }
}

APlayerPawn* URovenhellGameInstance::GetPlayerOfOwner(uint64 sessionId)
{
    if (!Players.Contains(sessionId))
    {
        UE_LOG(LogTemp, Warning, TEXT("해당 플레이어 폰 객체를 찾을 수 없습니다."));
        return nullptr;
    }
    else if (!Players[sessionId].IsValid())
    {
        while (!PlayersInfoLock.TryLock());
        UE_LOG(LogTemp, Warning, TEXT("해당 플레이어 폰 객체가 해제되었기 때문에 참조할 수 없습니다."));
        Players.Remove(sessionId); // 외부에서 해당 메모리가 해제되었을 경우
        PlayersInfoLock.Unlock();
        return nullptr;
    }
    return Players[sessionId].Get(); // Risky
}

TWeakObjectPtr<ANetHandler> URovenhellGameInstance::GetNetHandler()
{
    // 캐싱한 포인터가 없거나 유효하지 않은 경우
    if (NetHandlerWeakPtr == nullptr || !NetHandlerWeakPtr.IsValid())
    {
        ANetHandler* netHandler = Cast<ANetHandler>(UGameplayStatics::GetActorOfClass(this, ANetHandler::StaticClass()));
        if (!netHandler)
        {
            UE_LOG(LogTemp, Error, TEXT("현재 월드에서 NetHandler를 찾을 수 없습니다."));
            NetHandlerWeakPtr = nullptr;
        }
        else
        {
            NetHandlerWeakPtr = TWeakObjectPtr<ANetHandler>(netHandler); // 캐싱
        }
    }
    return NetHandlerWeakPtr;
}

TWeakObjectPtr<ANetActorSpawner> URovenhellGameInstance::GetNetActorSpawner()
{
    // 캐싱한 포인터가 없거나 유효하지 않은 경우
    if (NetActorSpawnerWeakPtr == nullptr || !NetActorSpawnerWeakPtr.IsValid())
    {
        ANetActorSpawner* netSpawner = Cast<ANetActorSpawner>(UGameplayStatics::GetActorOfClass(this, ANetActorSpawner::StaticClass()));
        if (!netSpawner)
        {
            UE_LOG(LogTemp, Error, TEXT("현재 월드에서 NetActorSpawner를 찾을 수 없습니다."));
            NetActorSpawnerWeakPtr = nullptr;
        }
        else
        {
            NetActorSpawnerWeakPtr = TWeakObjectPtr<ANetActorSpawner>(netSpawner); // 캐싱
        }
    }
    return NetActorSpawnerWeakPtr;
}
