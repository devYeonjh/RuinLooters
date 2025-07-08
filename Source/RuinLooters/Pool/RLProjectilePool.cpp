// Fill out your copyright notice in the Description page of Project Settings.

#include "RLProjectilePool.h"
#include "../Projectile/RLProjectile.h"
#include "Engine/World.h"
#include "TimerManager.h"

URLProjectilePool::URLProjectilePool()
{
    MaxPoolSize = 5;
    WorldRef = nullptr;
    ProjectileClassRef = nullptr;
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

    AvailableProjectiles.Empty();
    ActiveProjectiles.Empty();
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