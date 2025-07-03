// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RGAttackAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGAttackAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Notify 함수를 오버라이드
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};
