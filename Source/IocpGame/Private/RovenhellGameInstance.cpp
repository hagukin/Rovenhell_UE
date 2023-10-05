// Fill out your copyright notice in the Description page of Project Settings.


#include "RovenhellGameInstance.h"

URovenhellGameInstance::URovenhellGameInstance()
{
	GameExecType = MakeShared<ExecType>();
	TickCounter = MakeUnique<GameTickCounter>();
}

bool URovenhellGameInstance::AddPlayerController(uint64 sessionId, APlayerController* playerController)
{
    if (!Players.Contains(sessionId))
    {
        while (!PlayersInfoLock.TryLock());
        Players.Add(sessionId, TWeakObjectPtr<APlayerController>(playerController));
        PlayersInfoLock.Unlock();
    }
    else
    {
        while (!PlayersInfoLock.TryLock());
        UE_LOG(LogTemp, Warning, TEXT("세션 아이디 %i번 플레이어에 대한 정보가 이미 존재합니다. 값을 덮어 씁니다."), sessionId);
        Players[sessionId] = playerController;
        PlayersInfoLock.Unlock();
    }
    return true;
}

bool URovenhellGameInstance::RemovePlayerController(uint64 sessionId)
{
    if (Players.Contains(sessionId))
    {
        while (!PlayersInfoLock.TryLock());
        Players.Remove(sessionId);
        PlayersInfoLock.Unlock();
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 아이디 %i번 플레이어에 대한 정보가 존재하지 않아 삭제할 수 없습니다."), sessionId);
        return false;
    }
}

APlayerController* URovenhellGameInstance::GetPlayerControllerOfOwner(uint64 sessionId)
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