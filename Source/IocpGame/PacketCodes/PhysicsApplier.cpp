// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsApplier.h"
#include "MyUtility.h"

PhysicsApplier::PhysicsApplier()
{
}

PhysicsApplier::~PhysicsApplier()
{
}

bool PhysicsApplier::Init(TSharedPtr<NetSession> session, UGameInstance* gameInstance)
{
	if (!PacketApplier::Init(session, gameInstance)) return false;
	return true;
}

bool PhysicsApplier::ApplyPacket(TSharedPtr<RecvBuffer> packet, TSharedPtr<SerializeManager> deserializer)
{
	deserializer->Clear();
	SD_Actor* actorData = new SD_Actor();
	deserializer->ReadDataFromBuffer(packet);
	deserializer->DeserializeActor(actorData);
	UE_LOG(LogTemp, Warning, TEXT("%f %f %f / %f %f %f / %f %f %f"), actorData->xLoc, actorData->yLoc, actorData->zLoc, actorData->xRot, actorData->yRot, actorData->zRot, actorData->xVel, actorData->yVel, actorData->zVel);
	return true;
}
