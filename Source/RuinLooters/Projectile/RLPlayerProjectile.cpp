#include "RLPlayerProjectile.h"
#include "Engine/DamageEvents.h"
#include "../Character/RLCharacterEnemy.h"
#include "../Character/RLCharacterEnemyDragon.h"
#include "../Character/RLCharacterPlayer.h"

ARLPlayerProjectile::ARLPlayerProjectile()
{
    // 기본 타입만 설정 (실제 설정은 SetupWithPlayerSettings에서)
    ProjectileSettings.ProjectileType = EProjectileType::PlayerProjectile;
    ProjectileSettings.bCanPierceEnemies = true;
    ProjectileSettings.MaxPierceCount = 10;
    ProjectileSettings.LifeTime = 7.0f;

    ParticleSystem->SetRelativeLocation(FVector(0.f, 0.f, -300.f));
    
    UE_LOG(LogTemp, Log, TEXT("Player Projectile created"));
}

void ARLPlayerProjectile::SetupWithPlayerSettings(float CollisionRadius, float CollisionHeight, int32 Damage, float Speed)
{
    FProjectileSettings PlayerSettings;
    PlayerSettings.ProjectileType = EProjectileType::PlayerProjectile;
    PlayerSettings.Damage = Damage;
    PlayerSettings.Speed = Speed;
    PlayerSettings.LifeTime = 7.0f;
    PlayerSettings.CollisionRadius = CollisionRadius;  // 캐릭터에서 설정한 값
    PlayerSettings.CollisionHeight = CollisionHeight;  // 캐릭터에서 설정한 값
    PlayerSettings.bCanPierceEnemies = true;
    PlayerSettings.MaxPierceCount = 10;
    
    // 설정 적용
    ApplyProjectileSettings(PlayerSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Player Projectile setup - Radius: %f, Height: %f, Damage: %d, Speed: %f"), 
           CollisionRadius, CollisionHeight, Damage, Speed);
}

void ARLPlayerProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    // 플레이어 투사체는 플레이어 타입과 충돌하지 않음
    if (Cast<ARLCharacterPlayer>(OtherActor))
    {
        return;
    }

    // 지형(WorldStatic)에 충돌했는지 확인
    if (OtherComponent && OtherComponent->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic)
    {
        UE_LOG(LogTemp, Log, TEXT("Player Projectile hit terrain/world static - returning to pool"));
        
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

    // 유효한 타겟인지 확인 (적에게만 데미지)
    if (IsValidTarget(OtherActor))
    {
        if (!HitActors.Contains(OtherActor))
        {
            ApplyDamageToTarget(OtherActor);
            HitActors.Add(OtherActor);
            CurrentPierceCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Player Projectile hit enemy %s - continuing (pierce count: %d)"), 
                   *OtherActor->GetName(), CurrentPierceCount);
        }
        
        // 플레이어 투사체는 무조건 관통하여 계속 진행
        return;
    }
}

void ARLPlayerProjectile::ApplyDamageToTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // 일반 적 타겟
          if (ARLCharacterEnemy* Enemy = Cast<ARLCharacterEnemy>(Target))
      {
         				FDamageEvent DamageEvent;
				Enemy->TakeDamage((float)ProjectileSettings.Damage, DamageEvent, nullptr, this);
        UE_LOG(LogTemp, Log, TEXT("Player Projectile Hit Enemy: %s, Damage: %d, Pierce Count: %d"), 
               *Enemy->GetName(), ProjectileSettings.Damage, CurrentPierceCount);
    }
    // 드래곤 타겟
          else if (ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(Target))
      {
         				FDamageEvent DamageEvent;
				Dragon->TakeDamage((float)ProjectileSettings.Damage, DamageEvent, nullptr, this);
        UE_LOG(LogTemp, Log, TEXT("Player Projectile Hit Dragon: %s, Damage: %d, Pierce Count: %d"), 
               *Dragon->GetName(), ProjectileSettings.Damage, CurrentPierceCount);
    }
}

bool ARLPlayerProjectile::IsValidTarget(AActor* Target)
{
    if (!Target)
    {
        return false;
    }

    // 플레이어 투사체는 적에게만 데미지 (드래곤 포함)
    return Cast<ARLCharacterEnemy>(Target) || Cast<ARLCharacterEnemyDragon>(Target);
} 