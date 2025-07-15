// Fill out your copyright notice in the Description page of Project Settings.

#include "RLArrowPool.h"
#include "Projectile/RLArrow.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

URLArrowPool::URLArrowPool()
{
    ArrowClass = nullptr;
    World = nullptr;
}

void URLArrowPool::InitializeArrowPool(UWorld* InWorld, TSubclassOf<ARLArrow> InArrowClass, int32 InitialPoolSize)
{
    if (!InWorld || !InArrowClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLArrowPool::InitializeArrowPool - Invalid parameters"));
        return;
    }

    World = InWorld;
    ArrowClass = InArrowClass;

    // 기존 풀 정리
    ClearPool();

    // 화살 풀 초기화 (지정된 개수만큼 미리 생성)
    for (int32 i = 0; i < InitialPoolSize; ++i)
    {
        ARLArrow* Arrow = World->SpawnActor<ARLArrow>(ArrowClass);
        if (Arrow)
        {
            Arrow->DeactivateArrow();
            ArrowPool.Add(Arrow);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Arrow pool initialized with %d arrows"), ArrowPool.Num());
}

ARLArrow* URLArrowPool::GetArrowFromPool()
{
    if (!World || !ArrowClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("URLArrowPool::GetArrowFromPool - Pool not initialized"));
        return nullptr;
    }

    if (ArrowPool.Num() > 0)
    {
        // 풀에서 화살 가져오기
        ARLArrow* Arrow = ArrowPool.Pop();
        Arrow->ActivateArrow();
        ActiveArrows.Add(Arrow);
        UE_LOG(LogTemp, Log, TEXT("Arrow retrieved from pool. Pool size: %d"), ArrowPool.Num());
        return Arrow;
    }
    else
    {
        // 풀이 비어있으면 새로 생성
        ARLArrow* Arrow = World->SpawnActor<ARLArrow>(ArrowClass);
        if (Arrow)
        {
            Arrow->ActivateArrow();
            ActiveArrows.Add(Arrow);
            UE_LOG(LogTemp, Warning, TEXT("Pool empty, created new arrow"));
            return Arrow;
        }
    }

    return nullptr;
}

void URLArrowPool::ReturnArrowToPool(ARLArrow* Arrow)
{
    if (!Arrow)
    {
        return;
    }

    // 활성 화살 목록에서 제거
    ActiveArrows.Remove(Arrow);

    // 화살 비활성화
    Arrow->DeactivateArrow();

    // 풀에 반환
    ArrowPool.Add(Arrow);

    UE_LOG(LogTemp, Log, TEXT("Arrow returned to pool. Pool size: %d"), ArrowPool.Num());
}

void URLArrowPool::ClearPool()
{
    // 모든 화살 소멸
    for (ARLArrow* Arrow : ArrowPool)
    {
        if (Arrow && IsValid(Arrow))
        {
            Arrow->Destroy();
        }
    }

    for (ARLArrow* Arrow : ActiveArrows)
    {
        if (Arrow && IsValid(Arrow))
        {
            Arrow->Destroy();
        }
    }

    // 배열 비우기
    ArrowPool.Empty();
    ActiveArrows.Empty();

    UE_LOG(LogTemp, Log, TEXT("Arrow pool cleared"));
} 