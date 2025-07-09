#pragma once

#include "CoreMinimal.h"
#include "RLProjectile.h"
#include "RLDragonProjectile.generated.h"

/**
 * 드래곤 전용 투사체 클래스
 * 플레이어에게만 데미지를 주고, 관통하지 않음
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

protected:
	// 충돌 이벤트 오버라이드
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	// 데미지 적용 오버라이드 (드래곤 투사체 전용)
	virtual void ApplyDamageToTarget(AActor* Target) override;

	// 유효한 타겟인지 확인
	virtual bool IsValidTarget(AActor* Target) override;
}; 