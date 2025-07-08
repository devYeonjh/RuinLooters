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
public:
	FOnCharacterDie CharacterDie;

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
	
	

public:
	// FORCEINLINE을 이용한 Get, Set 함수 정의
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
	virtual void Attack() override;

	virtual void CallAttackCollision() override;

	void SwordAttackLineTrace();

	void PlayAttackSound();

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
};



