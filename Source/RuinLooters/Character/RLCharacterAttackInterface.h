// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RLCharacterAttackInterface.generated.h"

// 공격 완료 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnAttackCompleted);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URLCharacterAttackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RUINLOOTERS_API IRLCharacterAttackInterface
{
	GENERATED_BODY()

public:
	virtual void Attack();

	virtual void CallAttackCollision();
	
	virtual bool IsCanAttack() const;
	
	// 공격 완료 델리게이트 접근자
	virtual FOnAttackCompleted& GetOnAttackCompleted() = 0;
};
