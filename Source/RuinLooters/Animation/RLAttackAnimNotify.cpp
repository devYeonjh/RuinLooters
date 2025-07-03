// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RLAttackAnimNotify.h"
#include "Character/RLCharacterBase.h"

void URLAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);

    if (!MeshComp) return;

    // ARLCharacterBase Ÿ������ Ȯ��
    if (ARLCharacterBase* Character = Cast<ARLCharacterBase>(MeshComp->GetOwner()))
    {
        // ���� Ʈ���̽� �߻� �� ���� ���� ���?
        Character->SwordAttackLineTrace();
        Character->PlayAttackSound();
    }
}



