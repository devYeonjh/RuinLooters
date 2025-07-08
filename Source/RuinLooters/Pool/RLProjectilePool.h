// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "RLProjectilePool.generated.h"

class ARLProjectile;

/**
 * 투사체 오브젝트 풀링 클래스
 * 통합된 ARLProjectile을 사용하여 투사체들을 미리 생성하고 재사용하여 성능을 최적화합니다.
 */
UCLASS()
class RUINLOOTERS_API URLProjectilePool : public UObject
{
	GENERATED_BODY()

public:
	URLProjectilePool();

	// 풀 초기화
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void InitializePool(UWorld* World, TSubclassOf<ARLProjectile> ProjectileClass, int32 PoolSize = 20);

	// 투사체 가져오기
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	ARLProjectile* GetProjectile();

	// 투사체 반환
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnProjectile(ARLProjectile* Projectile);

	// 풀 정리
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ClearPool();

protected:
	// 사용 가능한 투사체들
	UPROPERTY()
	TArray<ARLProjectile*> AvailableProjectiles;

	// 사용 중인 투사체들
	UPROPERTY()
	TArray<ARLProjectile*> ActiveProjectiles;

	// 최대 풀 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
	int32 MaxPoolSize;

	// 월드 레퍼런스
	UPROPERTY()
	UWorld* WorldRef;

	// 투사체 클래스
	UPROPERTY()
	TSubclassOf<ARLProjectile> ProjectileClassRef;

private:
	// 새 투사체 생성
	ARLProjectile* CreateNewProjectile();

public:
	// Getter 함수들
	FORCEINLINE int32 GetAvailableCount() const { return AvailableProjectiles.Num(); }
	FORCEINLINE int32 GetActiveCount() const { return ActiveProjectiles.Num(); }
	FORCEINLINE int32 GetMaxPoolSize() const { return MaxPoolSize; }
}; 