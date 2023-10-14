// Copyright 2023 Haguk Kim
// Author: Haguk Kim

#include "NetHandler.h"
#include "PlayerPawn.h"

// Sets default values
ANetHandler::ANetHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANetHandler::BeginPlay()
{
	Super::BeginPlay();
	Init();
	GetRovenhellGameInstance()->GetNetHandler(); // 최초 캐싱
}

// Called every frame
void ANetHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (HostType)
	{
		case HostTypeEnum::CLIENT:
		case HostTypeEnum::CLIENT_HEADLESS:
			{
				Tick_UEClient(DeltaTime);
				break;
			}
		case HostTypeEnum::LOGIC_SERVER:
		case HostTypeEnum::LOGIC_SERVER_HEADLESS:
			{
				Tick_UEServer(DeltaTime);
				break;
			}
		case HostTypeEnum::NONE:
		default:
			{
				UE_LOG(LogTemp, Fatal, TEXT("호스트 타입이 설정되지 않았거나, 혹은 호스트 타입을 알 수 없습니다."));
				return;
			}
	}
}

void ANetHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Session->Kill();
	Super::EndPlay(EndPlayReason);
}

void ANetHandler::Init()
{
	// 호스트 타입 캐시 (초기화 이후 변하지 않으므로)
	InitGameHostType();
	if (HostType == HostTypeEnum::NONE)
	{
		UE_LOG(LogTemp, Fatal, TEXT("호스트 타입이 초기화되지 않았습니다."));
	}

	// 세션 생성
	Session = MakeShared<NetSession>();
	Session->Init(HostType);

	// 커넥션
	int32 port = 0;

	// 직렬화
	Serializer = MakeShared<SerializeManager>();
	Serializer->Init();
	Deserializer = MakeShared<SerializeManager>();
	Deserializer->Init();

	// Applier 생성
	InApplier = MakeUnique<InputApplier>();
	InApplier->Init(GetSessionShared(), GetGameInstance());
	ChatApplier = MakeUnique<ChatPacketApplier>();
	ChatApplier->Init(GetSessionShared(), GetGameInstance());
	GameApplier = MakeUnique<GameStateApplier>();
	GameApplier->Init(GetSessionShared(), GetGameInstance());
	MiddleApplier = MakeUnique<MiddlemanPacketApplier>();
	MiddleApplier->Init(GetSessionShared(), GetGameInstance());


	// 호스트 타입 별 초기화
	if (HostType == HostTypeEnum::CLIENT || HostType == HostTypeEnum::CLIENT_HEADLESS)
	{
		port = 7777;
	}
	else if (HostType == HostTypeEnum::LOGIC_SERVER || HostType == HostTypeEnum::LOGIC_SERVER_HEADLESS)
	{
		port = 8888;
		// TODO: HEADLESS 분리
		//GEngine->GameViewport->bDisableWorldRendering = 1;
	}
	
	NetAddress serverAddr(TEXT("127.0.0.1"), port); //////// TESTING

	bool connected = Session->TryConnect(serverAddr, 0, 1); // TODO: 만약 일정시간동안 시도를 반복하는 방식을 사용할 경우, BeginPlay에서 이를 처리하는 것은 좋지 못함, 비동기적 처리가 필요
	UE_LOG(LogTemp, Log, TEXT("커넥션 포트 번호: %i, 커넥션 상태: %d"), port, (int)connected);

	// 세션 작동
	if (Session->Start()) { UE_LOG(LogTemp, Log, TEXT("세션이 정상적으로 작동중입니다.")); }
}

void ANetHandler::UpdateLastProcessedInputTickForSession(uint64 sessionId, uint32 tick)
{
	if (!LastProcessedInputTick.Contains(sessionId))
	{
		LastProcessedInputTick.Add(sessionId, tick); // 없을 경우 새 항목 추가
	}
	LastProcessedInputTick[sessionId] = tick;
}

uint32 ANetHandler::GetLastProcessedInputTickForSession(uint64 sessionId)
{
	if (!LastProcessedInputTick.Contains(sessionId))
	{
		UE_LOG(LogTemp, Warning, TEXT("%i번 세션의 LastProcessedInputTick 정보를 찾을 수 없습니다, 기본값으로 새로 추가합니다."), sessionId);
		LastProcessedInputTick.Add(sessionId, 0); // 새 항목 추가
	}
	return LastProcessedInputTick[sessionId];
}

void ANetHandler::FillPacketSenderTypeHeader(TSharedPtr<SendBuffer> buffer)
{
	buffer->GetHeader()->senderType = HostType;
}

bool ANetHandler::DistributePendingPacket()
{
	// RecvPending 버퍼를 적절한 Applier로 전달한다
	const PacketHeader header = *RecvPending->GetHeader();

	bool applied = false;
	switch (header.type)
	{
	case PacketType::GAME_INPUT:
		{
			applied = InApplier->ApplyPacket(RecvPending, this);
			break;
		}
	case PacketType::CHAT_GLOBAL:
		{
			applied = ChatApplier->ApplyPacket(RecvPending, this);
			break;
		}
	case PacketType::GAME_STATE:
		{
			applied = GameApplier->ApplyPacket(RecvPending, this);
			break;
		}
	case PacketType::SESSION_INFO:
	case PacketType::SESSION_CONNECTED:
	case PacketType::SESSION_DISCONNECTED:
		{
			applied = MiddleApplier->ApplyPacket(RecvPending, this);
			break;
		}
	default:
		{
			UE_LOG(LogTemp, Warning, TEXT("패킷 type %i는 Applier에 의해 처리되지 않았습니다."), header.type);
			break;
		}
	}
	return applied;
}

uint8 ANetHandler::GenerateUniquePacketId()
{
	PacketUniqueId = (PacketUniqueId + 1) % UINT8_MAX;
	if (PacketUniqueId == 0) PacketUniqueId++;
	return PacketUniqueId;
}

void ANetHandler::InitGameHostType()
{
	URovenhellGameInstance* game = Cast<URovenhellGameInstance, UGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())); // downcasting
	if (game)
	{
		if (game->GetExecType())
		{
			HostType = game->GetExecType()->GetHostType();
		}
	}
}

void ANetHandler::Tick_UEClient(float DeltaTime)
{
	AccumulatedTickTime += DeltaTime;
	if (AccumulatedTickTime >= CONSUME_HISTORY_BUFFER_CYCLE) // 인터벌마다 Send
	{
		AccumulatedTickTime = 0;
		RegisterSend_UEClient(DeltaTime);
	}
	ProcessRecv_UEClient(DeltaTime);
}

void ANetHandler::RegisterSend_UEClient(float DeltaTime)
{
	APlayerPawn* Player = Cast<APlayerPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	SD_GameInputHistory* inputHistory = new SD_GameInputHistory(Player->GetGameInputPendings());

	Serializer->Clear();
	Serializer->Serialize((SD_Data*)inputHistory);

	TSharedPtr<SendBuffer> writeBuf = nullptr;
	while (!writeBuf) writeBuf = Session->BufManager->SendPool->PopBuffer();

	// NOTE: 클라이언트는 Scatter Gather을 사용하지 않고 단일 패킷으로 보낸다
	// 1) 서버만큼 방대한 데이터를 보낼 필요가 없기도 하고
	// 2) Scatter-Gather 사용시 서버에 막대한 부담이 가해지기 때문이다
	FillPacketSenderTypeHeader(writeBuf);
	writeBuf->GetHeader()->uniqueId = GenerateUniquePacketId();
	writeBuf->GetHeader()->packetOrder = 0;
	writeBuf->GetHeader()->fragmentCount = 1;
	writeBuf->GetHeader()->senderId = GetSessionShared()->GetSessionId();
	writeBuf->GetHeader()->protocol = PacketProtocol::CLIENT_ALLOW_MULTIPLE_PER_TICK;
	writeBuf->GetHeader()->type = PacketType::GAME_INPUT;
	Serializer->WriteDataToBuffer(writeBuf, Serializer->Array->GetData(), Serializer->Array->Num());
	Session->PushSendQueue(writeBuf);

	Player->ClearGameInputPendings(); // 사용이 끝난 인풋 히스토리를 전부 Flush한다
}

void ANetHandler::GetPendingBuffer_UEClient(float DeltaTime)
{
	while (!Session->Receiver->Lock.TryLock());
	for (auto& pair : Session->Receiver->PendingClientBuffers)
	{
		if (!pair.Value->IsEmpty())
		{
			pair.Value->Dequeue(RecvPending);
			break; // NOTE: 현재는 서버가 하나라는 전제 하에 바로 break 중이지만, 추후 만일 분산서버 연산을 처리할 경우 수정이 필요함.
		}
	}
	Session->Receiver->Lock.Unlock();
}

void ANetHandler::ProcessRecv_UEClient(float DeltaTime)
{
	while (!RecvPending)
	{
		GetPendingBuffer_UEClient(DeltaTime);
		if (!RecvPending) break; // 모든 세션으로부터 어떠한 대기 패킷도 없을 경우

		uint16 recvBufferProtocol = RecvPending->GetHeader()->protocol;
		switch (recvBufferProtocol)
		{
			case PacketProtocol::LOGIC_EVENT:
			case PacketProtocol::MIDDLEMAN_EVENT:
			{
				break;
			}
			// 클라이언트가 처리할 수 없는 프로토콜
			case PacketProtocol::NO_PROTOCOL:
			case PacketProtocol::CLIENT_ONCE_PER_TICK:
			case PacketProtocol::CLIENT_ALLOW_MULTIPLE_PER_TICK:
			{
				UE_LOG(LogTemp, Warning, TEXT("클라이언트에서 처리할 수 없는 잘못된 패킷 프로토콜 %i입니다. 패킷을 무시합니다."), recvBufferProtocol);
				Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
				RecvPending = nullptr; // 다시 루프
				break;
			}
		}
	}
	if (RecvPending)
	{
		if (!DistributePendingPacket())
		{
			UE_LOG(LogTemp, Error, TEXT("수신 완료한 패킷 내용을 적용하는 과정에서 문제가 발생했습니다, 해당 패킷 내용은 무시됩니다."));
		}
		Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
		RecvPending = nullptr;
	}
}

void ANetHandler::StartingNewGameTick_UEServer()
{
	if (!RecvPendings.IsEmpty() || RecvPending)
	{
		UE_LOG(LogTemp, Error, TEXT("마지막 게임 틱에서 처리되지 못한 패킷이 남아 있습니다. 해당 패킷은 삭제됩니다."));
	}
	RecvPendings.Empty();
	RecvPending = nullptr;

	for (auto& element : HasProcessedOncePerTickPacket)
	{
		element.Value = false;
		// TODO: 접속 해제된 세션 항목 삭제
	}
}

void ANetHandler::Tick_UEServer(float DeltaTime)
{
	StartingNewGameTick_UEServer();
	GetPendingBuffer_UEServer(DeltaTime);
	ProcessRecv_UEServer(DeltaTime);

	AccumulatedTickTime += DeltaTime;
	if (AccumulatedTickTime >= DESIRED_SERVER_BROADCAST_TIME) // 인터벌마다 Send
	{
		AccumulatedTickTime = 0;
		RegisterSend_UEServer(DeltaTime);
	}
}

void ANetHandler::GetPendingBuffer_UEServer(float DeltaTime)
{
	// 이번 틱에 처리할 패킷들을 세션별로 하나 이상씩 가져온다
	while (!Session->Receiver->Lock.TryLock());
	for (auto& pair : Session->Receiver->PendingClientBuffers)
	{
		while (!pair.Value->IsEmpty())
		{
			AddToRecvPendings = nullptr;
			pair.Value->Dequeue(AddToRecvPendings);
			RecvPendings.Enqueue(AddToRecvPendings);

			// 1틱 당 복수 처리가 허용된 패킷의 경우에는 계속 꺼내온다
			if (AddToRecvPendings->GetHeader()->protocol == PacketProtocol::CLIENT_ONCE_PER_TICK)
				break;
		}
	}
	Session->Receiver->Lock.Unlock();
}

void ANetHandler::ProcessRecv_UEServer(float DeltaTime)
{
	//uint32 packetCount = 0; // Stress test
	while (!RecvPendings.IsEmpty() || RecvPending)
	{
		if (RecvPending)
		{
			//PacketDebug(DeltaTime);
			if (!DistributePendingPacket())
			{
				UE_LOG(LogTemp, Error, TEXT("수신 완료한 패킷 내용을 적용하는 과정에서 문제가 발생했습니다, 해당 패킷 내용은 무시됩니다."));
			}

			//packetCount++;  // Stress test

			Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
			RecvPending = nullptr;
		}
		if (!RecvPendings.IsEmpty())
		{
			RecvPendings.Dequeue(RecvPending);

			uint64 sessionId = RecvPending->GetHeader()->senderId;
			uint16 recvBufferProtocol = RecvPending->GetHeader()->protocol;
			switch (recvBufferProtocol)
			{
				// 틱에 여러 번 처리 가능
				case PacketProtocol::CLIENT_ALLOW_MULTIPLE_PER_TICK:
				case PacketProtocol::MIDDLEMAN_EVENT:
				{
					break;
				}
				// 틱 당 한 번만 처리
				case PacketProtocol::CLIENT_ONCE_PER_TICK:
				{
					if (!HasProcessedOncePerTickPacket.Contains(sessionId))
					{
						HasProcessedOncePerTickPacket.Add(sessionId, false); // 새 항목 추가
					}

					if (*HasProcessedOncePerTickPacket.Find(sessionId))
					{
						UE_LOG(LogTemp, Warning, TEXT("알 수 없는 이유로 동일한 세션에서 틱당 한 번 처리해야 하는 패킷을 여러 차례 처리하려 시도 중입니다. 해당 패킷은 무시됩니다."));
						Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
						RecvPending = nullptr;
					}
					else
					{
						HasProcessedOncePerTickPacket[sessionId] = true;
					}
					break;
				}
				// 로직 서버가 처리할 수 없는 프로토콜
				case PacketProtocol::NO_PROTOCOL:
				case PacketProtocol::LOGIC_EVENT:
				{
					UE_LOG(LogTemp, Warning, TEXT("로직서버에서 처리할 수 없는 잘못된 패킷 프로토콜 %i입니다. 패킷을 무시합니다."), recvBufferProtocol);
					Session->BufManager->RecvPool->PushBuffer(MoveTemp(RecvPending));
					RecvPending = nullptr;
					break;
				}
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("로직서버 틱당 패킷 %i개 처리, 초당 %f개 처리"), packetCount, packetCount / DeltaTime); // Stress test
}

void ANetHandler::RegisterSend_UEServer(float DeltaTime)
{
	SD_GameState* gameStateData = new SD_GameState(
		GetWorld()->GetGameState(),
		GetRovenhellGameInstance()->TickCounter->GetTick(),
		DeltaTime,
		GetLastProcessedInputTicks()
	);

	// 모든 플레이어들에 대해서 물리 정보를 갱신한다.
	// TODO: 변경된 플레이어에 대해서만 정보를 보내는 것으로 패킷 크기 축소 가능
	for (const auto& element : GetRovenhellGameInstance()->GetPlayers())
	{
		gameStateData->AddPlayerPhysics(new SD_PawnPhysics(element.Key, element.Value.Get()));
	}

	// Scattering Packet
	Serializer->Clear();
	Serializer->Serialize((SD_Data*)gameStateData);
	uint8 uniqueId = GenerateUniquePacketId();
	uint8 totalFragmentCount = (uint8)FMath::CeilToInt((float)Serializer->Array->Num() / (NetBufferManager::SendBufferSize - sizeof(PacketHeader)));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("전송 패킷 갯수: %i"), totalFragmentCount)); // TESTING

	int packetCount = 0;
	for (int i = 0; i < Serializer->Array->Num(); i += NetBufferManager::SendBufferSize - sizeof(PacketHeader))
	{
		TSharedPtr<SendBuffer> writeBuf = nullptr;
		while (!writeBuf) writeBuf = Session->BufManager->SendPool->PopBuffer();

		FillPacketSenderTypeHeader(writeBuf);
		writeBuf->GetHeader()->uniqueId = uniqueId;
		writeBuf->GetHeader()->packetOrder = packetCount;
		writeBuf->GetHeader()->fragmentCount = totalFragmentCount;
		writeBuf->GetHeader()->senderId = Session->GetSessionId();
		writeBuf->GetHeader()->protocol = PacketProtocol::LOGIC_EVENT;
		writeBuf->GetHeader()->type = PacketType::GAME_STATE;

		if (Serializer->WriteDataToBuffer(writeBuf, Serializer->Array->GetData() + i, NetBufferManager::SendBufferSize - sizeof(PacketHeader)))
		{
			packetCount++;
			Session->PushSendQueue(writeBuf);
		}
	}
}
