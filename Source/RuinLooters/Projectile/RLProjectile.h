// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "RLProjectile.generated.h"

// 투사체 타입 열거형
UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	PlayerProjectile		UMETA(DisplayName = "Player Projectile"),
	DragonBreath			UMETA(DisplayName = "Dragon Breath"),
	Generic					UMETA(DisplayName = "Generic")
};

// 투사체 설정 구조체
USTRUCT(BlueprintType)
struct FProjectileSettings
{
	GENERATED_BODY()

	// 기본 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Settings")
	int32 Damage = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Settings")
	float Speed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Settings")
	float LifeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Settings")
	float CollisionRadius = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Settings")
	float CollisionHeight = 50.0f;

	// 관통 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pierce Settings")
	bool bCanPierceEnemies = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pierce Settings")
	int32 MaxPierceCount = 1;

	// 투사체 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings")
	EProjectileType ProjectileType = EProjectileType::Generic;

	FProjectileSettings()
	{
		Damage = 50;
		Speed = 2000.0f;
		LifeTime = 5.0f;
		CollisionRadius = 25.0f;
		CollisionHeight = 50.0f;
		bCanPierceEnemies = false;
		MaxPierceCount = 1;
		ProjectileType = EProjectileType::Generic;
	}
};

UCLASS()
class RUINLOOTERS_API ARLProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ARLProjectile();

protected:
	// 콜리전 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleCollision;

	// 파티클 시스템 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* ParticleSystem;

	// 투사체 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 투사체 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings")
	FProjectileSettings ProjectileSettings;

	// 오브젝트 풀 사용 여부
	UPROPERTY()
	bool bIsPooled;

	// 타이머 핸들
	FTimerHandle LifeTimerHandle;

	// 관통 관련 런타임 데이터
	UPROPERTY()
	int32 CurrentPierceCount;

	// 이미 히트한 적들을 추적
	UPROPERTY()
	TArray<AActor*> HitActors;

public:	
	virtual void BeginPlay() override;

	// 투사체 초기화 (오브젝트 풀에서 사용)
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitializeProjectile(FVector StartLocation, FVector Direction, const FProjectileSettings& Settings);

	// 간단한 초기화 (기존 호환성)
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitializeProjectileSimple(FVector StartLocation, FVector Direction, int32 Damage, float Speed);

	// 투사체 비활성화 (오브젝트 풀로 반환)
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void DeactivateProjectile();

	// 투사체 설정 적용
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ApplyProjectileSettings(const FProjectileSettings& Settings);

	// 프리셋 설정 함수들 (제거됨 - 각 파생 클래스에서 처리)

	// 충돌 이벤트 (가상 함수로 변경)
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Getter/Setter
	FORCEINLINE const FProjectileSettings& GetProjectileSettings() const { return ProjectileSettings; }
	FORCEINLINE void SetProjectileSettings(const FProjectileSettings& Settings) { ProjectileSettings = Settings; }
	FORCEINLINE bool IsPooled() const { return bIsPooled; }
	FORCEINLINE void SetPooled(bool InPooled) { bIsPooled = InPooled; }
	FORCEINLINE int32 GetCurrentPierceCount() const { return CurrentPierceCount; }
	FORCEINLINE EProjectileType GetProjectileType() const { return ProjectileSettings.ProjectileType; }

protected:
	// 생존 시간 만료시 호출
	UFUNCTION()
	void OnLifeTimeExpired();

	// 관통 가능한지 확인
	bool CanPierceTarget(AActor* Target);

	// 데미지 적용 (타입에 따라 다른 처리) - 가상 함수로 변경
	virtual void ApplyDamageToTarget(AActor* Target);

	// 유효한 타겟인지 확인 - 새로 추가된 가상 함수
	virtual bool IsValidTarget(AActor* Target);
}; 