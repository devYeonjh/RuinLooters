// Fill out your copyright notice in the Description page of Project Settings.

#include "RLProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../Character/RLCharacterBase.h"
#include "../Character/RLCharacterEnemy.h"
#include "../Character/RLCharacterEnemyDragon.h"
#include "../Character/RLCharacterPlayer.h"

ARLProjectile::ARLProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    // 콜리전 컴포넌트 설정
    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    RootComponent = SphereCollision;
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    SphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    SphereCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

    // 파티클 시스템 컴포넌트 설정
    ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));
    ParticleSystem->SetupAttachment(RootComponent);

    // 투사체 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = SphereCollision;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // 기본 설정 초기화
    ProjectileSettings = FProjectileSettings();
    bIsPooled = false;
    CurrentPierceCount = 0;

    // 충돌 이벤트 바인딩
    SphereCollision->OnComponentHit.AddDynamic(this, &ARLProjectile::OnHit);
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
    
    // 콜리전 크기 설정
    SphereCollision->SetSphereRadius(Settings.CollisionRadius);
    
    // 투사체 이동 컴포넌트 설정
    ProjectileMovement->InitialSpeed = Settings.Speed;
    ProjectileMovement->MaxSpeed = Settings.Speed;
}

void ARLProjectile::SetupAsPlayerProjectile()
{
    FProjectileSettings PlayerSettings;
    PlayerSettings.ProjectileType = EProjectileType::PlayerProjectile;
    PlayerSettings.Damage = 40;
    PlayerSettings.Speed = 3000.0f;
    PlayerSettings.LifeTime = 3.0f;
    PlayerSettings.CollisionRadius = 15.0f;
    PlayerSettings.bCanPierceEnemies = true;
    PlayerSettings.MaxPierceCount = 3;
    
    ApplyProjectileSettings(PlayerSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Projectile setup as Player Projectile"));
}

void ARLProjectile::SetupAsDragonBreath()
{
    FProjectileSettings DragonSettings;
    DragonSettings.ProjectileType = EProjectileType::DragonBreath;
    DragonSettings.Damage = 75;
    DragonSettings.Speed = 1500.0f;
    DragonSettings.LifeTime = 5.0f;
    DragonSettings.CollisionRadius = 25.0f;
    DragonSettings.bCanPierceEnemies = false;
    DragonSettings.MaxPierceCount = 1;
    
    ApplyProjectileSettings(DragonSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Projectile setup as Dragon Breath"));
}

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
    SetActorLocation(FVector::ZeroVector);
    SetActorRotation(FRotator::ZeroRotator);
}

void ARLProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    // 적/플레이어 구분하여 데미지 적용
    bool bIsValidTarget = false;
    bool bShouldContinue = false;
    
    // 플레이어 투사체인 경우 - 적에게만 데미지
    if (ProjectileSettings.ProjectileType == EProjectileType::PlayerProjectile)
    {
        if (Cast<ARLCharacterEnemy>(OtherActor) || Cast<ARLCharacterEnemyDragon>(OtherActor))
        {
            if (!HitActors.Contains(OtherActor))
            {
                ApplyDamageToTarget(OtherActor);
                HitActors.Add(OtherActor);
                CurrentPierceCount++;
                bIsValidTarget = true;
                
                // 관통 가능한지 확인
                if (ProjectileSettings.bCanPierceEnemies && CurrentPierceCount < ProjectileSettings.MaxPierceCount)
                {
                    bShouldContinue = true;
                }
            }
        }
    }
    // 드래곤 투사체인 경우 - 플레이어에게만 데미지
    else if (ProjectileSettings.ProjectileType == EProjectileType::DragonBreath)
    {
        if (Cast<ARLCharacterPlayer>(OtherActor))
        {
            if (!HitActors.Contains(OtherActor))
            {
                ApplyDamageToTarget(OtherActor);
                HitActors.Add(OtherActor);
                CurrentPierceCount++;
                bIsValidTarget = true;
            }
        }
    }
    // 범용 투사체인 경우 - 소유자가 아닌 모든 캐릭터에게 데미지
    else
    {
        if (Cast<ARLCharacterBase>(OtherActor))
        {
            if (!HitActors.Contains(OtherActor))
            {
                ApplyDamageToTarget(OtherActor);
                HitActors.Add(OtherActor);
                CurrentPierceCount++;
                bIsValidTarget = true;
                
                // 관통 가능한지 확인
                if (ProjectileSettings.bCanPierceEnemies && CurrentPierceCount < ProjectileSettings.MaxPierceCount)
                {
                    bShouldContinue = true;
                }
            }
        }
    }

    // 관통 가능하면 계속 진행
    if (bShouldContinue)
    {
        return;
    }

    // 벽이나 관통 한계에 도달했을 때
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
        Player->TakeCharacterDamage(ProjectileSettings.Damage);
        UE_LOG(LogTemp, Log, TEXT("%s Projectile Hit Player: %s, Damage: %d"), 
               ProjectileSettings.ProjectileType == EProjectileType::DragonBreath ? TEXT("Dragon") : TEXT("Player"),
               *Player->GetName(), ProjectileSettings.Damage);
    }
    // 일반 적 타겟
    else if (ARLCharacterEnemy* Enemy = Cast<ARLCharacterEnemy>(Target))
    {
        Enemy->TakeCharacterDamage(ProjectileSettings.Damage);
        UE_LOG(LogTemp, Log, TEXT("Player Projectile Hit Enemy: %s, Damage: %d, Pierce Count: %d"), 
               *Enemy->GetName(), ProjectileSettings.Damage, CurrentPierceCount);
    }
    // 드래곤 타겟
    else if (ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(Target))
    {
        Dragon->TakeDragonDamage(ProjectileSettings.Damage);
        UE_LOG(LogTemp, Log, TEXT("Player Projectile Hit Dragon: %s, Damage: %d, Pierce Count: %d"), 
               *Dragon->GetName(), ProjectileSettings.Damage, CurrentPierceCount);
    }
    // 기본 캐릭터 타겟
    else if (ARLCharacterBase* Character = Cast<ARLCharacterBase>(Target))
    {
        Character->TakeCharacterDamage(ProjectileSettings.Damage);
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