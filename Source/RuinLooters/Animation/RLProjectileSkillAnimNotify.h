// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RLProjectileSkillAnimNotify.generated.h"

/**
 * 투사체 스킬 발사를 위한 애님 노티파이 클래스
 * 애니메이션의 특정 타이밍에 투사체를 발사합니다.
 */
UCLASS()
class RUINLOOTERS_API URLProjectileSkillAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	// Notify 함수의 오버라이드
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
}; 