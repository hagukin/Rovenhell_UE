// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class IOCPGAME_API NetMemory
{
public:
	NetMemory();
	~NetMemory();

	template<typename Type, typename... Args>
	Type* Allocate(Args&&... args);
};