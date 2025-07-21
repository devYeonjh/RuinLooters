// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/RLProjectileSkillAnimNotify.h"
#include "Character/RLCharacterPlayer.h"
#include "Character/RLCharacterEnemyDragon.h"

void URLProjectileSkillAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    // ARLCharacterPlayer 타입인지 확인
    if (ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(MeshComp->GetOwner()))
    {
        // 투사체 발사 함수 호출
        Player->FirePlayerProjectile();
        UE_LOG(LogTemp, Log, TEXT("ProjectileSkillAnimNotify: Fired player projectile"));
    }
    // ARLCharacterEnemyDragon 타입인지 확인
    else if (ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(MeshComp->GetOwner()))
    {
        // 브레스 투사체 발사 함수 호출
        Dragon->FireBreathProjectile();
        UE_LOG(LogTemp, Log, TEXT("ProjectileSkillAnimNotify: Fired dragon breath projectile"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectileSkillAnimNotify: Owner is neither a player character nor an enemy dragon"));
    }
} 