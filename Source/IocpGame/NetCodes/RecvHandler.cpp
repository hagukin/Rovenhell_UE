// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "RecvHandler.h"

RecvHandler::RecvHandler(HostTypeEnum hostType)
{
    HostType = hostType;
}

RecvHandler::~RecvHandler()
{
}

bool RecvHandler::Init()
{
    return true;
}

uint32 RecvHandler::Run()
{
    FPlatformProcess::Sleep(0.3f); // 초기화 대기
    TSharedPtr<RecvBuffer> recvBuffer;
    float count = 0;
    while (StopCounter.GetValue() == 0)
    {
        while (!recvBuffer)
        {
            recvBuffer = Session->BufManager->RecvPool->PopBuffer();
        }
        count = 0;
        if (!Session->Recv(recvBuffer))
        {
            // 수신 못했을 경우 새로 재시도, 이때 버퍼에 일부 데이터가 채워졌을 가능성이 있기 때문에 버퍼 리셋
            recvBuffer->Reset();
            continue; 
        }
        else
        {
            // Recv 성공
            if (HostType == HostTypeEnum::CLIENT || HostType == HostTypeEnum::CLIENT_HEADLESS)
            {
                FPlatformProcess::YieldThread(); // 최적화 위해 더 호출이 잦은 타 스레드에게 양보
            }
        }
        Session->PushRecvQueue(MoveTemp(recvBuffer));
        recvBuffer = nullptr;
    }
    return 0;
}

void RecvHandler::Stop()
{
    Thread->Suspend(true);
    StopCounter.Increment(); // Run 루프 중단시킴, 다음번 iteration에서 return 0;
    Thread->Suspend(false);
}

void RecvHandler::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Recv 스레드가 정상적으로 종료되었습니다."));
}

FRunnableThread* RecvHandler::GetThread()
{
    return Thread;
}

bool RecvHandler::CreateThread()
{
    // 로직 시작점
    Thread = FRunnableThread::Create(this, TEXT("RecvHandler Thread")); // 여기서 FRunnable::Init() -> FRunnable::Run() 호출
    return (Thread != nullptr);
}

void RecvHandler::SetSession(NetSession* session)
{
    Session = session;
}
