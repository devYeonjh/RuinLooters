#include "RLDragonProjectile.h"
#include "Engine/DamageEvents.h"
#include "../Character/RLCharacterPlayer.h"
#include "../Character/RLCharacterEnemyDragon.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

ARLDragonProjectile::ARLDragonProjectile()
{
    // 기본 타입만 설정 (실제 설정은 SetupWithDragonSettings에서)
    ProjectileSettings.ProjectileType = EProjectileType::DragonBreath;
    ProjectileSettings.bCanPierceEnemies = false;
    ProjectileSettings.MaxPierceCount = 1;
    ProjectileSettings.LifeTime = 5.0f;
    
    // 폭발 파티클 템플릿 초기화
    ExplosionParticleTemplate = nullptr;
    
    // 나이아가라 폭발 이펙트 초기화
    ExplosionNiagaraEffect = nullptr;
    
    // Block 충돌 이벤트 바인딩 (지형과의 충돌 처리용)
    if (CapsuleCollision)
    {
        // 지형과의 Block 충돌만 명시적으로 설정
        CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
        // 폰(플레이어, 적)과는 Overlap 유지
        CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        // 활과의 충돌 방지를 위해 WorldDynamic 채널 무시
        CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile created with hit detection"));
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

void ARLDragonProjectile::BeginPlay()
{
    Super::BeginPlay();

    CapsuleCollision->OnComponentHit.AddDynamic(this, &ARLDragonProjectile::OnHit);
    CapsuleCollision->OnComponentBeginOverlap.AddDynamic(this, &ARLDragonProjectile::OnOverlap);
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

    // 지형(WorldStatic)과의 충돌은 OnHit에서 처리됨 (Block 충돌)
    // 여기서는 Overlap 충돌만 처리

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
        
        // 현재 캡슐 콜리전 위치 저장
        FVector CollisionLocation = GetActorLocation();
        
        // 폭발 파티클 생성 (플레이어와 충돌 시)
        CreateExplosionEffect(CollisionLocation);
        
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
         				FDamageEvent DamageEvent;
				Player->TakeDamage((float)ProjectileSettings.Damage, DamageEvent, nullptr, this);
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

void ARLDragonProjectile::CreateNiagaraExplosionEffect(FVector Location)
{
    if (!ExplosionNiagaraEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Niagara explosion effect set for Dragon Projectile"));
        return;
    }

    // 나이아가라 폭발 이펙트를 해당 위치에 생성 (한 사이클 후 완전 파괴)
    UNiagaraComponent* SpawnedNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        ExplosionNiagaraEffect,
        Location - FVector(0.0f, 0.0f, 110.0f),
        FRotator::ZeroRotator,
        FVector(1.0f), // 기본 스케일
        true,  // Auto destroy when finished
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    // 나이아가라 컴포넌트가 생성되었다면 강제로 완료 후 파괴 보장
    if (SpawnedNiagaraComponent)
    {
        // 루핑이 비활성화되어 있는지 확인하고 강제로 한 번만 재생하도록 설정
        SpawnedNiagaraComponent->SetAutoDestroy(true);
        SpawnedNiagaraComponent->SetNiagaraVariableBool(TEXT("User.Loop"), false);
        
        // 최대 5초 후에 강제로 파괴되도록 타이머 설정
        FTimerHandle DestroyTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, [SpawnedNiagaraComponent]()
        {
            if (IsValid(SpawnedNiagaraComponent))
            {
                SpawnedNiagaraComponent->DestroyComponent();
            }
        }, 5.0f, false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile Niagara explosion effect spawned at location: %s"), *Location.ToString());
}

void ARLDragonProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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

    // 지형(WorldStatic)이나 다른 Block 객체와 충돌했을 때 처리
    if (OtherComp && OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic)
    {
        UE_LOG(LogTemp, Log, TEXT("Dragon Projectile hit terrain/world static - creating explosion at hit point"));
        
        // 충돌 지점에 폭발 파티클 생성
        CreateExplosionEffect(Hit.Location);
        
        // 나이아가라 폭발 이펙트 생성 (풀로 돌아가기 전)
        CreateNiagaraExplosionEffect(Hit.Location);
        
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

    // 다른 Block 객체와 충돌했을 때도 폭발 생성 (벽, 장애물 등)
    UE_LOG(LogTemp, Log, TEXT("Dragon Projectile hit block object: %s - creating explosion"), *OtherActor->GetName());
} 