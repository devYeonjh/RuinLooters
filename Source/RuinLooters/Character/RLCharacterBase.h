// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuinLootersCharacter.h"
#include "RLEnumRepository.h"
#include "RLCharacterBase.generated.h"

// ĳ���� ��� ��������Ʈ
DECLARE_MULTICAST_DELEGATE(FOnDie);

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLCharacterBase : public ARuinLootersCharacter
{
	GENERATED_BODY()
	
public:
	ARLCharacterBase();
public:
	FOnDie CharacterDie;

protected:
	// ĳ���� ����
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

	// ���� WeaponName
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CharacterWeaponName;

	// ���� Ȱ �� ���� ������Ʈ �� ����� Enum
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType Form;

	// ���� �ִϸ��̼�
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* CurrentMontage;

	// ��� �ִϸ��̼�
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* DieMontage;

	UPROPERTY()
	class UAnimInstance* AnimInstance;
public:
	// ���� ���ð����� Ȯ��
	uint8 bIsCanAttack : 1;
protected:
	// �� ���Ͽ� ���� ����
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMeshComponent;

	// ������ ���̺��� ���� ��
	struct FWeaponTableRow* RowWeapon;

	// ������ ���̺��� ���� �̸�
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FName WeaponRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
	int32 Money;

	UPROPERTY()
	class URLGameInstance* GameInstance;

	UPROPERTY()
	class UWorld* World;

	// ����, �ǰ�, ��� ����
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DieSound;
public:
	// 콤보 시스템 상태 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	int32 CurrentComboStep = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	int32 ComboMaxStep = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bComboInput = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bCanNextCombo = false;

	// FORCEINLINE ̿ Get, Set Լ 
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

	// 콤보 섹션으로 점프하는 함수
	void PlayComboMontage(int32 ComboStep);

protected:
	virtual void BeginPlay() override;

	// Attack Ÿ   Ÿ̸ 
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void ApplyWeaponAbility(struct FWeaponTableRow* ApplyWeapon);
};



