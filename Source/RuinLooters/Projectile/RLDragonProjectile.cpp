#include "RLDragonProjectile.h"
#include "../Character/RLCharacterPlayer.h"
#include "../Character/RLCharacterEnemyDragon.h"
#include "../Pool/RLProjectilePool.h"
#include "Kismet/GameplayStatics.h"

ARLDragonProjectile::ARLDragonProjectile()
{
    // 기본 타입만 설정 (실제 설정은 SetupWithDragonSettings에서)
    ProjectileSettings.ProjectileType = EProjectileType::DragonBreath;
    ProjectileSettings.bCanPierceEnemies = false;
    ProjectileSettings.MaxPierceCount = 1;
    ProjectileSettings.LifeTime = 5.0f;
    
    // 폭발 파티클 템플릿 초기화
    ExplosionParticleTemplate = nullptr;
    
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

void ARLDragonProjectile::SetExplosionParticleTemplate(UParticleSystem* InExplosionTemplate)
{
    ExplosionParticleTemplate = InExplosionTemplate;
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile explosion particle template set"));
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
        UE_LOG(LogTemp, Log, TEXT("Dragon Projectile hit terrain/world static - creating explosion"));
        
        // 폭발 파티클 생성
        CreateExplosionEffect(GetActorLocation());
        
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
        
        // 폭발 파티클 생성 (플레이어와 충돌 시)
        CreateExplosionEffect(GetActorLocation());
        
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



void ARLDragonProjectile::CreateExplosionEffect(FVector Location)
{
    if (!ExplosionParticleTemplate)
    {
        UE_LOG(LogTemp, Warning, TEXT("No explosion particle template set for Dragon Projectile"));
        return;
    }

    // 폭발 파티클을 직접 생성 (발사체 파티클과 별개)
    UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(), 
        ExplosionParticleTemplate, 
        Location, 
        FRotator::ZeroRotator,
        FVector(1.0f), // 기본 스케일
        true  // Auto destroy when finished
    );
    
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile explosion particle spawned at location: %s"), *Location.ToString());
} 