#include "RLDragonProjectile.h"
#include "../Character/RLCharacterPlayer.h"
#include "../Character/RLCharacterEnemyDragon.h"

ARLDragonProjectile::ARLDragonProjectile()
{
    // 기본 타입만 설정 (실제 설정은 SetupWithDragonSettings에서)
    ProjectileSettings.ProjectileType = EProjectileType::DragonBreath;
    ProjectileSettings.bCanPierceEnemies = false;
    ProjectileSettings.MaxPierceCount = 1;
    ProjectileSettings.LifeTime = 5.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile created"));
}

void ARLDragonProjectile::SetupWithDragonSettings(float CollisionRadius, float CollisionHeight, int32 Damage, float Speed)
{
    FProjectileSettings DragonSettings;
    DragonSettings.ProjectileType = EProjectileType::DragonBreath;
    DragonSettings.Damage = Damage;
    DragonSettings.Speed = Speed;
    DragonSettings.LifeTime = 5.0f;
    DragonSettings.CollisionRadius = CollisionRadius;  // 캐릭터에서 설정한 값
    DragonSettings.CollisionHeight = CollisionHeight;  // 캐릭터에서 설정한 값
    DragonSettings.bCanPierceEnemies = false;
    DragonSettings.MaxPierceCount = 1;
    
    // 설정 적용
    ApplyProjectileSettings(DragonSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile setup - Radius: %f, Height: %f, Damage: %d, Speed: %f"), 
           CollisionRadius, CollisionHeight, Damage, Speed);
}

void ARLDragonProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    // 드래곤 투사체는 드래곤 타입과 충돌하지 않음
    if (Cast<ARLCharacterEnemyDragon>(OtherActor))
    {
        return;
    }

    // 지형(WorldStatic)에 충돌했는지 확인
    if (OtherComponent && OtherComponent->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic)
    {
        UE_LOG(LogTemp, Log, TEXT("Dragon Projectile hit terrain/world static - returning to pool"));
        
        // 파티클 시스템 정지
        if (ParticleSystem)
        {
            ParticleSystem->Deactivate();
        }

        // 오브젝트 풀로 반환 또는 삭제
        if (bIsPooled)
        {
            OnLifeTimeExpired();
        }
        else
        {
            Destroy();
        }
        return;
    }

    // 유효한 타겟인지 확인 (플레이어에게만 데미지)
    if (IsValidTarget(OtherActor))
    {
        if (!HitActors.Contains(OtherActor))
        {
            ApplyDamageToTarget(OtherActor);
            HitActors.Add(OtherActor);
            CurrentPierceCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Dragon Projectile hit player %s - stopping"), 
                   *OtherActor->GetName());
        }
        
        // 드래곤 투사체는 플레이어에게 맞으면 정지
        if (ParticleSystem)
        {
            ParticleSystem->Deactivate();
        }

        if (bIsPooled)
        {
            OnLifeTimeExpired();
        }
        else
        {
            Destroy();
        }
        return;
    }
}

void ARLDragonProjectile::ApplyDamageToTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // 플레이어 타겟
    if (ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(Target))
    {
        Player->TakeCharacterDamage(ProjectileSettings.Damage);
        UE_LOG(LogTemp, Log, TEXT("Dragon Projectile Hit Player: %s, Damage: %d"), 
               *Player->GetName(), ProjectileSettings.Damage);
    }
}

bool ARLDragonProjectile::IsValidTarget(AActor* Target)
{
    if (!Target)
    {
        return false;
    }

    // 드래곤 투사체는 플레이어에게만 데미지
    return Cast<ARLCharacterPlayer>(Target) != nullptr;
} 