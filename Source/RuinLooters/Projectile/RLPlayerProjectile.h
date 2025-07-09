#pragma once

#include "CoreMinimal.h"
#include "RLProjectile.h"
#include "RLPlayerProjectile.generated.h"

/**
 * 플레이어 전용 투사체 클래스
 * 적에게만 데미지를 주고, 관통 가능
 */
UCLASS()
class RUINLOOTERS_API ARLPlayerProjectile : public ARLProjectile
{
	GENERATED_BODY()

public:
	ARLPlayerProjectile();

	// 플레이어 설정으로 투사체 초기화
	UFUNCTION(BlueprintCallable, Category = "Player Projectile")
	void SetupWithPlayerSettings(float CollisionRadius, float CollisionHeight, int32 Damage = 40, float Speed = 3000.0f);

protected:
	// 충돌 이벤트 오버라이드
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	// 데미지 적용 오버라이드 (플레이어 투사체 전용)
	virtual void ApplyDamageToTarget(AActor* Target) override;

	// 유효한 타겟인지 확인
	virtual bool IsValidTarget(AActor* Target) override;
}; 