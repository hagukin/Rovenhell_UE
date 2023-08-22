// Fill out your copyright notice in the Description page of Project Settings.


#include "NetMemory.h"

NetMemory::NetMemory()
{
}

NetMemory::~NetMemory()
{
}

template<typename Type, typename... Args>
Type* NetMemory::Allocate(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	new(memory)Type(forward<Args>(args)...); // memory 위치에 placement new, 이때 생성자 아규먼트도 전달
	return memory;
}