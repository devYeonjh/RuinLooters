// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RGAttackAnimNotify.h"
#include "Character/RGCharacterBase.h"

void URGAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);

    if (!MeshComp) return;

    // ARGCharacterBase 타입인지 확인
    if (ARGCharacterBase* Character = Cast<ARGCharacterBase>(MeshComp->GetOwner()))
    {
        // 라인 트레이스 발생 및 공격 사운드 재생
        Character->SwordAttackLineTrace();
        Character->PlayAttackSound();
    }
}
