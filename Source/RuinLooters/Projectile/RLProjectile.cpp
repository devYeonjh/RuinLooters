// Fill out your copyright notice in the Description page of Project Settings.

#include "RLProjectile.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../Character/RLCharacterBase.h"
#include "../Character/RLCharacterEnemy.h"
#include "../Character/RLCharacterEnemyDragon.h"
#include "../Character/RLCharacterPlayer.h"
#include "Engine/DamageEvents.h"

ARLProjectile::ARLProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    // 콜리전 컴포넌트 설정
    CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
    RootComponent = CapsuleCollision;
    CapsuleCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CapsuleCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CapsuleCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

    // 파티클 시스템 컴포넌트 설정
    ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));
    ParticleSystem->SetupAttachment(RootComponent);
    // 파티클 시스템의 충돌 완전 비활성화 - 오직 SphereCollision만 충돌 담당
    ParticleSystem->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ParticleSystem->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

    // 투사체 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CapsuleCollision;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // 기본 설정 초기화
    ProjectileSettings = FProjectileSettings();
    bIsPooled = false;
    CurrentPierceCount = 0;

    // 충돌 이벤트 바인딩
    CapsuleCollision->OnComponentBeginOverlap.AddDynamic(this, &ARLProjectile::OnOverlap);
}

void ARLProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    // 기본 설정 적용
    ApplyProjectileSettings(ProjectileSettings);
}

void ARLProjectile::InitializeProjectile(FVector StartLocation, FVector Direction, const FProjectileSettings& Settings)
{
    // 위치 및 회전 설정
    SetActorLocation(StartLocation);
    SetActorRotation(Direction.Rotation());

    // 투사체 설정 적용
    ApplyProjectileSettings(Settings);

    // 투사체 이동 설정
    ProjectileMovement->InitialSpeed = Settings.Speed;
    ProjectileMovement->MaxSpeed = Settings.Speed;
    ProjectileMovement->Velocity = Direction * Settings.Speed;

    // 파티클 시스템 재시작
    if (ParticleSystem)
    {
        ParticleSystem->Activate(true);
    }

    // 관통 카운트 리셋
    CurrentPierceCount = 0;
    HitActors.Empty();

    // 생존 시간 타이머 시작
    if (Settings.LifeTime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &ARLProjectile::OnLifeTimeExpired, Settings.LifeTime, false);
    }

    UE_LOG(LogTemp, Log, TEXT("Projectile initialized - Type: %d, Damage: %d, Speed: %f"), 
           (int32)Settings.ProjectileType, Settings.Damage, Settings.Speed);
}

void ARLProjectile::InitializeProjectileSimple(FVector StartLocation, FVector Direction, int32 Damage, float Speed)
{
    FProjectileSettings Settings = ProjectileSettings;
    Settings.Damage = Damage;
    Settings.Speed = Speed;
    
    InitializeProjectile(StartLocation, Direction, Settings);
}

void ARLProjectile::ApplyProjectileSettings(const FProjectileSettings& Settings)
{
    ProjectileSettings = Settings;
    
    // 콜리전 크기 설정 (캡슐: 반지름, 높이)
    CapsuleCollision->SetCapsuleSize(Settings.CollisionRadius, Settings.CollisionHeight);
    
    // 투사체 이동 컴포넌트 설정
    ProjectileMovement->InitialSpeed = Settings.Speed;
    ProjectileMovement->MaxSpeed = Settings.Speed;
}

// SetupAsPlayerProjectile와 SetupAsDragonBreath 함수들이 제거됨
// 각 파생 클래스(RLPlayerProjectile, RLDragonProjectile)에서 처리

void ARLProjectile::DeactivateProjectile()
{
    // 타이머 정리
    GetWorldTimerManager().ClearTimer(LifeTimerHandle);

    // 투사체 이동 정지
    ProjectileMovement->Velocity = FVector::ZeroVector;

    // 파티클 시스템 정지
    if (ParticleSystem)
    {
        ParticleSystem->Deactivate();
    }

    // 관통 데이터 리셋
    CurrentPierceCount = 0;
    HitActors.Empty();

    // 위치 리셋
    SetActorLocation(FVector(0.0f, 0.0f, -1000.0f));
    SetActorRotation(FRotator::ZeroRotator);
}

void ARLProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    // 지형(WorldStatic)에 충돌했는지 확인
    if (OtherComponent && OtherComponent->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic)
    {
        UE_LOG(LogTemp, Log, TEXT("Projectile hit terrain/world static - returning to pool"));
        
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

    // 유효한 타겟인지 확인하고 데미지 적용
    if (IsValidTarget(OtherActor))
    {
        if (!HitActors.Contains(OtherActor))
        {
            ApplyDamageToTarget(OtherActor);
            HitActors.Add(OtherActor);
            CurrentPierceCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Projectile hit character %s - continuing (pierce count: %d)"), 
                   *OtherActor->GetName(), CurrentPierceCount);
        }
        
        // 무조건 관통하여 계속 진행
        return;
    }
}

void ARLProjectile::ApplyDamageToTarget(AActor* Target)
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
        UE_LOG(LogTemp, Log, TEXT("%s Projectile Hit Player: %s, Damage: %d"), 
               ProjectileSettings.ProjectileType == EProjectileType::DragonBreath ? TEXT("Dragon") : TEXT("Player"),
               *Player->GetName(), ProjectileSettings.Damage);
    }
    // 일반 적 타겟
    else if (ARLCharacterEnemy* Enemy = Cast<ARLCharacterEnemy>(Target))
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
    // 기본 캐릭터 타겟
    else if (ARLCharacterBase* Character = Cast<ARLCharacterBase>(Target))
    {
        				FDamageEvent DamageEvent;
				Character->TakeDamage((float)ProjectileSettings.Damage, DamageEvent, nullptr, this);
        UE_LOG(LogTemp, Log, TEXT("Projectile Hit Character: %s, Damage: %d"), 
               *Character->GetName(), ProjectileSettings.Damage);
    }
}

void ARLProjectile::OnLifeTimeExpired()
{
    // 타이머 정리
    GetWorldTimerManager().ClearTimer(LifeTimerHandle);

    if (bIsPooled)
    {
        // 오브젝트 풀로 반환
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        DeactivateProjectile();
    }
    else
    {
        // 일반 삭제
        Destroy();
    }
}

bool ARLProjectile::CanPierceTarget(AActor* Target)
{
    // 이미 맞은 적이 아니고, 관통 가능 상태이며, 관통 한계에 도달하지 않았는지 확인
    return ProjectileSettings.bCanPierceEnemies && 
           !HitActors.Contains(Target) && 
           CurrentPierceCount < ProjectileSettings.MaxPierceCount;
}

bool ARLProjectile::IsValidTarget(AActor* Target)
{
    // 기본 구현 - 기존 로직 유지 (타입별 구분)
    if (!Target)
    {
        return false;
    }

    // 플레이어 투사체인 경우 - 적에게만 데미지
    if (ProjectileSettings.ProjectileType == EProjectileType::PlayerProjectile)
    {
        return Cast<ARLCharacterEnemy>(Target) || Cast<ARLCharacterEnemyDragon>(Target);
    }
    // 드래곤 투사체인 경우 - 플레이어에게만 데미지
    else if (ProjectileSettings.ProjectileType == EProjectileType::DragonBreath)
    {
        return Cast<ARLCharacterPlayer>(Target) != nullptr;
    }
    // 범용 투사체인 경우 - 소유자가 아닌 모든 캐릭터에게 데미지
    else
    {
        return Cast<ARLCharacterBase>(Target) != nullptr;
    }
} 