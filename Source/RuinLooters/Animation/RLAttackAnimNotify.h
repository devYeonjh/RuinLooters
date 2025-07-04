// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RLAttackAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLAttackAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Notify �Լ��� �������̵�
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};



