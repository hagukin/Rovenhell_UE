// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "SendHandler.h"

SendHandler::SendHandler(HostTypeEnum hostType) : StopCounter(0)
{
    HostType = hostType;
}

SendHandler::~SendHandler()
{
}

bool SendHandler::Init()
{
    return true;
}

uint32 SendHandler::Run()
{
    FPlatformProcess::Sleep(0.3f); // 초기화 대기
    while (StopCounter.GetValue() == 0)
    {
        if (SendPending)
        {
            if (!Session->Send(SendPending))
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("버퍼 Send 실패")));
            }
            else
            {
                // Send 성공
                FPlatformProcess::YieldThread(); // 클라, 서버 모두 발송 주기가 틱보다 길기 때문에 최적화 위해 타 스레드에게 양보
            }
            Session->BufManager->SendPool->PushBuffer(MoveTemp(SendPending));
            SendPending = nullptr;
        }
        if (!SendQueue.IsEmpty())
        {
            SendQueue.Dequeue(SendPending);
        }
    }
    return 0;
}

void SendHandler::Stop()
{
    Thread->Suspend(true);
    StopCounter.Increment(); // Run 루프 중단시킴, 다음번 iteration에서 return 0;
    Thread->Suspend(false);
}

void SendHandler::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Send 스레드가 정상적으로 종료되었습니다."));
}

FRunnableThread* SendHandler::GetThread()
{
    return Thread;
}

bool SendHandler::CreateThread()
{
    // 로직 시작점
    Thread = FRunnableThread::Create(this, TEXT("SendHandler Thread")); // 여기서 FRunnable::Init() -> FRunnable::Run() 호출
    return (Thread != nullptr);
}

void SendHandler::SetSession(NetSession* session)
{
    Session = session;
}
