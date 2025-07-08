// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/RLProjectileSkillAnimNotify.h"
#include "Character/RLCharacterPlayer.h"

void URLProjectileSkillAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);

    if (!MeshComp) return;

    // ARLCharacterPlayer 타입인지 확인
    if (ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(MeshComp->GetOwner()))
    {
        // 투사체 발사 함수 호출
        Player->FirePlayerProjectile();
        UE_LOG(LogTemp, Log, TEXT("ProjectileSkillAnimNotify: Fired player projectile"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectileSkillAnimNotify: Owner is not a player character"));
    }
} 