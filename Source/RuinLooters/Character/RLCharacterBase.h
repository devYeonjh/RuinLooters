// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuinLootersCharacter.h"
#include "Character/RLCharacterAttackInterface.h"
#include "RLEnumRepository.h"
#include "RLPlayerComboAttackDataAsset.h"
#include "RLCharacterBase.generated.h"

// 캐릭터 죽음 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnCharacterDie);

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLCharacterBase : public ARuinLootersCharacter, public IRLCharacterAttackInterface
{
	GENERATED_BODY()
	
public:
	ARLCharacterBase();

	virtual void StartRoll();
public:
	FOnCharacterDie CharacterDie;

protected:
	// 캐릭터 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float CurrentHp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float MaxHp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Range;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float MontageSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 Defence;

	// 현재 WeaponName
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CharacterWeaponName;

	// 검과 활 등 무기 컴포넌트 및 폼타입 Enum
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType Form;

	// 공격 애니메이션
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* ComboActionMontage;

	// 죽음 애니메이션
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* DieMontage;

	UPROPERTY()
	class UAnimInstance* AnimInstance;

	// 콤보 공격 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	class URLPlayerComboAttackDataAsset* ComboActionData;

	// 손 소켓에 붙일 무기
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMeshComponent;

	// 데이터 테이블에서 무기 정보
	struct FWeaponTableRow* RowWeapon;

	// 데이터 테이블에서 무기 이름
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FName WeaponRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
	int32 Money;

	UPROPERTY()
	class URLGameInstance* GameInstance;

	UPROPERTY()
	class UWorld* World;

	// 공격, 피격, 죽음 사운드
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DieSound;
	
	// 파티클 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
	class UParticleSystem* HitParticleTemplate;


	// --- Roll(구르기) 시스템 추가 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* RollMontage;


	float OriginalMaxWalkSpeed = 0.f; // 구르기 전 원래 속도 저장

	bool bIsRolling = false;
	bool bIsInvincible = false;
	FVector RollDirection;

public:
	// FORCEINLINE을 이용한 Get, Set 함수 정의
	FORCEINLINE int32 GetAttackDamage() { return AttackDamage; };

	FORCEINLINE void SetAttackDamage(int32 NewAttackDamage) { AttackDamage = NewAttackDamage; };

	FORCEINLINE float GetCurrentHp() { return CurrentHp; };

	FORCEINLINE void SetCurrentHp(float NewCurrentHp) { CurrentHp = NewCurrentHp; };

	FORCEINLINE float GetMaxHp() { return MaxHp; };

	FORCEINLINE void SetMaxHp(float NewMaxHp) { MaxHp = NewMaxHp; };

	FORCEINLINE int32 GetMoney() const { return Money; };

	FORCEINLINE void SetMoney(int32 NewMoney) { Money = NewMoney; };

	FORCEINLINE int32 GetDefence() const { return Defence; };

	FORCEINLINE void SetDefence(int32 NewDefence) { Defence = NewDefence; };

	FORCEINLINE struct FWeaponTableRow* GetCharacterWeaponRow() { return RowWeapon; };

	FORCEINLINE void SetRowWeapon(FWeaponTableRow* NewRowWeapon) { RowWeapon = NewRowWeapon; };

	FORCEINLINE class USkeletalMeshComponent* GetCharacterWeaponMeshComponent() { return WeaponMeshComponent; };

	FORCEINLINE void SetWeaponMeshComponent(USkeletalMeshComponent* NewWeaponMeshComponent) { WeaponMeshComponent = NewWeaponMeshComponent; };

	FORCEINLINE FName GetCharacterWeaponName() { return CharacterWeaponName; };

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount);

	virtual void Die();

	void ChangeWeapon(struct FWeaponTableRow* ChangeWeapon);

	/** Called for looking input */
	virtual void Attack() override;

	virtual void CallAttackCollision() override;

	void PlayAttackSound();

	// 롤 몽타주 종료 콜백
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 플레이어 입력 차단/복원 함수 (가상 함수로 선언, 플레이어에서 오버라이드)
	virtual void DisablePlayerInput() {}
	virtual void EnablePlayerInput() {}

	UFUNCTION()
	void StartInvincible();
	UFUNCTION()
	void EndInvincible();

	virtual void Move(const FInputActionValue& Value) override;

protected:
	virtual void BeginPlay() override;

	void ApplyWeaponAbility(struct FWeaponTableRow* ApplyWeapon);

	// 콤보 관련 함수들
	void ProcessComboCommand();

	void ComboActionBegin();

	void ComboActionEnd(UAnimMontage* Montage, bool bInterrupted);

	void SetComboCheckTimer();

	void ComboCheck();

	int32 CurrentCombo = 0;

	FTimerHandle ComboTimerHandle;

	bool HasNextComboCommand = false;

	float AttackSpeedRate = 1.0f;
	// Tick 오버라이드
	virtual void Tick(float DeltaTime) override;
};



