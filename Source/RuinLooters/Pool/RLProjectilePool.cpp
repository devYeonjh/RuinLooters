// Fill out your copyright notice in the Description page of Project Settings.

#include "RLProjectilePool.h"
#include "../Projectile/RLProjectile.h"
#include "Engine/World.h"
#include "TimerManager.h"

URLProjectilePool::URLProjectilePool()
{
    MaxPoolSize = 10;
    MaxExplosionPoolSize = 5;  // 폭발 파티클은 작은 풀 크기
    WorldRef = nullptr;
    ProjectileClassRef = nullptr;
    ExplosionClassRef = nullptr;
}

void URLProjectilePool::InitializePool(UWorld* World, TSubclassOf<ARLProjectile> ProjectileClass, int32 PoolSize)
{
    if (!World || !ProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLProjectilePool::InitializePool - Invalid World or ProjectileClass"));
        return;
    }

    WorldRef = World;
    ProjectileClassRef = ProjectileClass;
    MaxPoolSize = PoolSize;

    // 기존 풀 정리
    ClearPool();

    // 초기 투사체들 생성
    for (int8 i = 0; i < MaxPoolSize; i++)
    {
        ARLProjectile* NewProjectile = CreateNewProjectile();
        if (NewProjectile)
        {
            NewProjectile->SetPooled(true);
            NewProjectile->SetActorHiddenInGame(true);
            NewProjectile->SetActorEnableCollision(false);
            AvailableProjectiles.Add(NewProjectile);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("URLProjectilePool::InitializePool - Pool initialized with %d projectiles"), AvailableProjectiles.Num());
}

ARLProjectile* URLProjectilePool::GetProjectile()
{
    if (!WorldRef || !ProjectileClassRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLProjectilePool::GetProjectile - Pool not initialized"));
        return nullptr;
    }

    ARLProjectile* Projectile = nullptr;

    // 사용 가능한 투사체가 있는지 확인
    if (AvailableProjectiles.Num() > 0)
    {
        Projectile = AvailableProjectiles.Pop();
    }
    else
    {
        // 사용 가능한 투사체가 없으면 새로 생성
        Projectile = CreateNewProjectile();
        if (Projectile)
        {
            Projectile->SetPooled(true);
            UE_LOG(LogTemp, Warning, TEXT("URLProjectilePool::GetProjectile - Pool exhausted, created new projectile"));
        }
    }

    if (Projectile)
    {
        ActiveProjectiles.Add(Projectile);
        Projectile->SetActorHiddenInGame(false);
        Projectile->SetActorEnableCollision(true);
    }

    return Projectile;
}

void URLProjectilePool::ReturnProjectile(ARLProjectile* Projectile)
{
    if (!Projectile || !Projectile->IsPooled())
    {
        return;
    }

    // 활성 목록에서 제거
    ActiveProjectiles.RemoveSingle(Projectile);

    // 투사체 비활성화
    Projectile->DeactivateProjectile();
    Projectile->SetActorHiddenInGame(true);
    Projectile->SetActorEnableCollision(false);

    // 사용 가능한 목록에 다시 추가
    AvailableProjectiles.Add(Projectile);
}

void URLProjectilePool::ClearPool()
{
    // 모든 투사체 제거
    for (ARLProjectile* Projectile : AvailableProjectiles)
    {
        if (Projectile && IsValid(Projectile))
        {
            Projectile->Destroy();
        }
    }

    for (ARLProjectile* Projectile : ActiveProjectiles)
    {
        if (Projectile && IsValid(Projectile))
        {
            Projectile->Destroy();
        }
    }

    // 폭발 파티클 제거
    for (ARLProjectile* ExplosionEffect : AvailableExplosionEffects)
    {
        if (ExplosionEffect && IsValid(ExplosionEffect))
        {
            ExplosionEffect->Destroy();
        }
    }

    for (ARLProjectile* ExplosionEffect : ActiveExplosionEffects)
    {
        if (ExplosionEffect && IsValid(ExplosionEffect))
        {
            ExplosionEffect->Destroy();
        }
    }

    AvailableProjectiles.Empty();
    ActiveProjectiles.Empty();
    AvailableExplosionEffects.Empty();
    ActiveExplosionEffects.Empty();
}

ARLProjectile* URLProjectilePool::CreateNewProjectile()
{
    if (!WorldRef || !ProjectileClassRef)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ARLProjectile* NewProjectile = WorldRef->SpawnActor<ARLProjectile>(
        ProjectileClassRef, 
        FVector::ZeroVector, 
        FRotator::ZeroRotator, 
        SpawnParams
    );

    return NewProjectile;
}

void URLProjectilePool::InitializeExplosionPool(UWorld* World, TSubclassOf<ARLProjectile> ExplosionClass, int32 PoolSize)
{
    if (!World || !ExplosionClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLProjectilePool::InitializeExplosionPool - Invalid World or ExplosionClass"));
        return;
    }

    WorldRef = World;
    ExplosionClassRef = ExplosionClass;
    MaxExplosionPoolSize = PoolSize;

    // 기존 폭발 파티클 풀 정리
    for (ARLProjectile* ExplosionEffect : AvailableExplosionEffects)
    {
        if (ExplosionEffect && IsValid(ExplosionEffect))
        {
            ExplosionEffect->Destroy();
        }
    }

    for (ARLProjectile* ExplosionEffect : ActiveExplosionEffects)
    {
        if (ExplosionEffect && IsValid(ExplosionEffect))
        {
            ExplosionEffect->Destroy();
        }
    }

    AvailableExplosionEffects.Empty();
    ActiveExplosionEffects.Empty();

    // 초기 폭발 파티클들 생성
    for (int8 i = 0; i < MaxExplosionPoolSize; i++)
    {
        ARLProjectile* NewExplosionEffect = CreateNewExplosionEffect();
        if (NewExplosionEffect)
        {
            NewExplosionEffect->SetPooled(true);
            NewExplosionEffect->SetActorHiddenInGame(true);
            NewExplosionEffect->SetActorEnableCollision(false);
            AvailableExplosionEffects.Add(NewExplosionEffect);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("URLProjectilePool::InitializeExplosionPool - Explosion pool initialized with %d effects"), AvailableExplosionEffects.Num());
}

ARLProjectile* URLProjectilePool::GetExplosionEffect()
{
    if (!WorldRef || !ExplosionClassRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLProjectilePool::GetExplosionEffect - Explosion pool not initialized"));
        return nullptr;
    }

    ARLProjectile* ExplosionEffect = nullptr;

    // 사용 가능한 폭발 파티클이 있는지 확인
    if (AvailableExplosionEffects.Num() > 0)
    {
        ExplosionEffect = AvailableExplosionEffects.Pop();
    }
    else
    {
        // 사용 가능한 폭발 파티클이 없으면 새로 생성
        ExplosionEffect = CreateNewExplosionEffect();
        if (ExplosionEffect)
        {
            ExplosionEffect->SetPooled(true);
            UE_LOG(LogTemp, Warning, TEXT("URLProjectilePool::GetExplosionEffect - Explosion pool exhausted, created new effect"));
        }
    }

    if (ExplosionEffect)
    {
        ActiveExplosionEffects.Add(ExplosionEffect);
        ExplosionEffect->SetActorHiddenInGame(false);
        ExplosionEffect->SetActorEnableCollision(false); // 폭발 파티클은 콜리전 비활성화
    }

    return ExplosionEffect;
}

void URLProjectilePool::ReturnExplosionEffect(ARLProjectile* ExplosionEffect)
{
    if (!ExplosionEffect || !ExplosionEffect->IsPooled())
    {
        return;
    }

    // 활성 목록에서 제거
    ActiveExplosionEffects.RemoveSingle(ExplosionEffect);

    // 폭발 파티클 비활성화
    ExplosionEffect->DeactivateProjectile();
    ExplosionEffect->SetActorHiddenInGame(true);
    ExplosionEffect->SetActorEnableCollision(false);

    // 사용 가능한 목록에 다시 추가
    AvailableExplosionEffects.Add(ExplosionEffect);
}

ARLProjectile* URLProjectilePool::CreateNewExplosionEffect()
{
    if (!WorldRef || !ExplosionClassRef)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ARLProjectile* NewExplosionEffect = WorldRef->SpawnActor<ARLProjectile>(
        ExplosionClassRef, 
        FVector::ZeroVector, 
        FRotator::ZeroRotator, 
        SpawnParams
    );

    return NewExplosionEffect;
} 