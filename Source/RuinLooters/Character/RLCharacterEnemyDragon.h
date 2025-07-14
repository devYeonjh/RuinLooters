// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/RLCharacterAttackInterface.h"
#include "Engine/Engine.h"
#include "RLCharacterEnemyDragon.generated.h"

// 드래곤 죽음 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnDragonDie);

/**
 * 드래곤 적 캐릭터 클래스
 * 기본적인 전투 기능과 캡슐 콜리전을 이용한 공격 기능을 포함합니다.
 */
UCLASS()
class RUINLOOTERS_API ARLCharacterEnemyDragon : public ACharacter, public IRLCharacterAttackInterface
{
	GENERATED_BODY()

public:
	ARLCharacterEnemyDragon();

	// 드래곤 죽음 델리게이트
	FOnDragonDie DragonDie;

protected:
	// 기본 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Stats")
	float CurrentHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Stats")
	float MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Stats")
	int32 AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Stats")
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Stats")
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Stats")
	int32 Defence;

	// 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* DieMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* GroundBreathMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* SkyBreathMontage;

	UPROPERTY()
	class UAnimInstance* AnimInstance;

	// 공격 가능 여부
	uint8 bIsCanAttack : 1;

	// 사운드
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DieSound;

	// 파티클 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
	class UParticleSystem* HitParticleTemplate;

	// 캡슐 콜리전 공격 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Attack")
	float CapsuleAttackRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Attack")
	float CapsuleAttackHeight;

	// 브레스 공격 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Breath")
	int32 BreathDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Breath")
	float BreathRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Breath")
	TSubclassOf<class ARLProjectile> ProjectileClass;

	// 폭발 파티클 템플릿 (투사체 충돌 시 생성되는 파티클)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Breath")
	class UParticleSystem* ExplosionParticleTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Breath")
	float BreathProjectileCollisionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Breath")
	float BreathProjectileCollisionHeight;

	// 투사체 풀링 시스템
	UPROPERTY()
	class URLProjectilePool* ProjectilePool;

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Getter 함수들
	FORCEINLINE float GetCurrentHp() const { return CurrentHp; }
	FORCEINLINE float GetMaxHp() const { return MaxHp; }
	FORCEINLINE int32 GetAttackDamage() const { return AttackDamage; }
	FORCEINLINE float GetRange() const { return Range; }
	FORCEINLINE float GetAttackSpeed() const { return AttackSpeed; }
	FORCEINLINE int32 GetDefence() const { return Defence; }

	// Setter 함수들
	FORCEINLINE void SetCurrentHp(float NewCurrentHp) { CurrentHp = NewCurrentHp; }
	FORCEINLINE void SetMaxHp(float NewMaxHp) { MaxHp = NewMaxHp; }
	FORCEINLINE void SetAttackDamage(int32 NewAttackDamage) { AttackDamage = NewAttackDamage; }
	FORCEINLINE void SetRange(float NewRange) { Range = NewRange; }
	FORCEINLINE void SetAttackSpeed(float NewAttackSpeed) { AttackSpeed = NewAttackSpeed; }
	FORCEINLINE void SetDefence(int32 NewDefence) { Defence = NewDefence; }

	// 전투 관련 함수들
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Heal(int32 HealAmount);

	virtual void Die();

	virtual void Attack() override;

	virtual void CallAttackCollision() override;

	// 브레스 공격 함수
	UFUNCTION(BlueprintCallable, Category = "Dragon Breath")
	void BreathAttack();

	// 브레스 투사체 발사
	UFUNCTION()
	void FireBreathProjectile();

	// 공격 사운드 재생
	void PlayAttackSound();

protected:
	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
