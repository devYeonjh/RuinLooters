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

	// 활성 발사체 자동 정리 (게임 종료 시)
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void CleanupActiveProjectiles();

	// 폭발 파티클 전용 풀 초기화
	UFUNCTION(BlueprintCallable, Category = "Explosion Pool")
	void InitializeExplosionPool(UWorld* World, TSubclassOf<ARLProjectile> ExplosionClass, int32 PoolSize = 10);

	// 폭발 파티클 가져오기
	UFUNCTION(BlueprintCallable, Category = "Explosion Pool")
	ARLProjectile* GetExplosionEffect();

	// 폭발 파티클 반환
	UFUNCTION(BlueprintCallable, Category = "Explosion Pool")
	void ReturnExplosionEffect(ARLProjectile* ExplosionEffect);

protected:
	// 사용 가능한 투사체들
	UPROPERTY()
	TArray<ARLProjectile*> AvailableProjectiles;

	// 사용 중인 투사체들
	UPROPERTY()
	TArray<ARLProjectile*> ActiveProjectiles;

	// 폭발 파티클 관련 (별도 풀)
	UPROPERTY()
	TArray<ARLProjectile*> AvailableExplosionEffects;

	UPROPERTY()
	TArray<ARLProjectile*> ActiveExplosionEffects;

	// 최대 풀 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
	int32 MaxPoolSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
	int32 MaxExplosionPoolSize;

	// 월드 레퍼런스
	UPROPERTY()
	UWorld* WorldRef;

	// 투사체 클래스
	UPROPERTY()
	TSubclassOf<ARLProjectile> ProjectileClassRef;

	// 폭발 파티클 클래스
	UPROPERTY()
	TSubclassOf<ARLProjectile> ExplosionClassRef;

private:
	// 새 투사체 생성
	ARLProjectile* CreateNewProjectile();

	// 새 폭발 파티클 생성
	ARLProjectile* CreateNewExplosionEffect();

public:
	// Getter 함수들
	FORCEINLINE int32 GetAvailableCount() const { return AvailableProjectiles.Num(); }
	FORCEINLINE int32 GetActiveCount() const { return ActiveProjectiles.Num(); }
	FORCEINLINE int32 GetMaxPoolSize() const { return MaxPoolSize; }
	
	// 폭발 파티클 Getter 함수들
	FORCEINLINE int32 GetAvailableExplosionCount() const { return AvailableExplosionEffects.Num(); }
	FORCEINLINE int32 GetActiveExplosionCount() const { return ActiveExplosionEffects.Num(); }
	FORCEINLINE int32 GetMaxExplosionPoolSize() const { return MaxExplosionPoolSize; }
}; 