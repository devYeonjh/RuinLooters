// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLCharacterAttackInterface.h"

// Add default functionality here for any IRLCharacterAttackInterface functions that are not pure virtual.

void IRLCharacterAttackInterface::Attack()
{
}

void IRLCharacterAttackInterface::CallAttackCollision()
{
}

bool IRLCharacterAttackInterface::IsCanAttack() const
{
	return true; // 기본적으로 공격 가능
}

FOnAttackCompleted& IRLCharacterAttackInterface::GetOnAttackCompleted()
{
	// 더미 델리게이트 반환 (기본 구현)
	static FOnAttackCompleted DummyDelegate;
	return DummyDelegate;
}
