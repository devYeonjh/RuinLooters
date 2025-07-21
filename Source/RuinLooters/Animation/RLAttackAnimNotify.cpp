// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RLAttackAnimNotify.h"
#include "Character/RLCharacterBase.h"
#include "Character/RLCharacterAttackInterface.h"

void URLAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    // ARLCharacterBase 타입인지 확인
    if (IRLCharacterAttackInterface* Character = Cast<IRLCharacterAttackInterface>(MeshComp->GetOwner()))
    {
        // 공격 트레이스 발생 및 사운드 재생
        Character->CallAttackCollision();
    }
}



