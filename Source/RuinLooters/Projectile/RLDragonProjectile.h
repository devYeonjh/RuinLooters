#pragma once

#include "CoreMinimal.h"
#include "RLProjectile.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "RLDragonProjectile.generated.h"

/**
 * 드래곤 전용 투사체 클래스
 * 플레이어에게만 데미지를 주고, 관통하지 않음
 * 충돌 시 폭발 파티클 효과 생성
 */
UCLASS()
class RUINLOOTERS_API ARLDragonProjectile : public ARLProjectile
{
	GENERATED_BODY()

public:
	ARLDragonProjectile();

	// 드래곤 설정으로 투사체 초기화
	UFUNCTION(BlueprintCallable, Category = "Dragon Projectile")
	void SetupWithDragonSettings(float CollisionRadius, float CollisionHeight, int32 Damage = 75, float Speed = 1500.0f);

	// 폭발 파티클 템플릿 설정
	UFUNCTION(BlueprintCallable, Category = "Dragon Projectile")
	void SetExplosionParticleTemplate(UParticleSystem* InExplosionTemplate);

protected:
	// 폭발 파티클 시스템 (발사체 파티클과 별개)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion Effects")
	UParticleSystem* ExplosionParticleTemplate;
	
	// 나이아가라 폭발 이펙트 (풀로 돌아가기 전 생성)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion Effects")
	UNiagaraSystem* ExplosionNiagaraEffect;
	// 충돌 이벤트 오버라이드
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	// Block 충돌 이벤트 (지형 충돌용)
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 데미지 적용 오버라이드 (드래곤 투사체 전용)
	virtual void ApplyDamageToTarget(AActor* Target) override;

	// 유효한 타겟인지 확인
	virtual bool IsValidTarget(AActor* Target) override;

	// 폭발 파티클 생성
	UFUNCTION(BlueprintCallable, Category = "Dragon Projectile")
	void CreateExplosionEffect(FVector Location);
	
	// 나이아가라 폭발 이펙트 생성 (풀로 돌아가기 전)
	UFUNCTION(BlueprintCallable, Category = "Dragon Projectile")
	void CreateNiagaraExplosionEffect(FVector Location);
}; 