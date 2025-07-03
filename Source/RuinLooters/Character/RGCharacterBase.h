// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuinLootersCharacter.h"
#include "RGEnumRepository.h"
#include "RGCharacterBase.generated.h"

// 캐릭터 사망 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnDie);

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARGCharacterBase : public ARuinLootersCharacter
{
	GENERATED_BODY()
	
public:
	ARGCharacterBase();
public:
	FOnDie CharacterDie;

protected:
	// 캐릭터 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 CurrentHp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 MaxHp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	float Range;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float AttackSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 Defence;

	// 소유 WeaponName
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CharacterWeaponName;

	// 추후 활 등 무기 업데이트 시 사용할 Enum
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType Form;

	// 공격 애니메이션
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* CurrentMontage;

	// 사망 애니메이션
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* DieMontage;

	UPROPERTY()
	class UAnimInstance* AnimInstance;

	// 공격 대기시간인지 확인
	uint8 bIsCanAttack : 1;

	// 손 소켓에 붙일 무기
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMeshComponent;

	// 데이터 테이블의 무기 열
	struct FWeaponTableRow* RowWeapon;

	// 데이터 테이블의 무기 이름
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FName WeaponRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
	int32 Money;

	UPROPERTY()
	class URGGameInstance* GameInstance;

	UPROPERTY()
	class UWorld* World;

	// 공격, 피격, 사망 사운드
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DieSound;
	

public:
	// FORCEINLINE을 이용해 Get, Set 함수 구현
	FORCEINLINE int32 GetAttackDamage() { return AttackDamage; };

	FORCEINLINE void SetAttackDamage(int32 NewAttackDamage) { AttackDamage = NewAttackDamage; };

	FORCEINLINE int32 GetCurrentHp() { return CurrentHp; };

	FORCEINLINE void SetCurrentHp(int32 NewCurrentHp) { CurrentHp = NewCurrentHp; };

	FORCEINLINE int32 GetMaxHp() { return MaxHp; };

	FORCEINLINE void SetMaxHp(int32 NewMaxHp) { MaxHp = NewMaxHp; };

	FORCEINLINE int32 GetMoney() const { return Money; };

	FORCEINLINE void SetMoney(int32 NewMoney) { Money = NewMoney; };

	FORCEINLINE int32 GetDefence() const { return Defence; };

	FORCEINLINE void SetDefence(int32 NewDefence) { Defence = NewDefence; };

	FORCEINLINE struct FWeaponTableRow* GetCharacterWeaponRow() { return RowWeapon; };

	FORCEINLINE void SetRowWeapon(FWeaponTableRow* NewRowWeapon) { RowWeapon = NewRowWeapon; };

	FORCEINLINE class USkeletalMeshComponent* GetCharacterWeaponMeshComponent() { return WeaponMeshComponent; };

	FORCEINLINE void SetWeaponMeshComponent(USkeletalMeshComponent* NewWeaponMeshComponent) { WeaponMeshComponent = NewWeaponMeshComponent; };

	FORCEINLINE FName GetCharacterWeaponName() { return CharacterWeaponName; };

	virtual void TakeCharacterDamage(int32 RecieveDamage);

	virtual void TakeCharacterHeal(int32 RecieveHealAmount);

	virtual void Die();

	void ChangeWeapon(struct FWeaponTableRow* ChangeWeapon);

	/** Called for looking input */
	void Attack();

	void SwordAttackLineTrace();

	void PlayAttackSound();

protected:
	virtual void BeginPlay() override;

	// Attack 몽타주 종료 시 타이머 정지
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void ApplyWeaponAbility(struct FWeaponTableRow* ApplyWeapon);
};
