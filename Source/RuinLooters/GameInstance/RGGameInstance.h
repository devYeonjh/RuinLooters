// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h" 
#include "RGGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FWeaponTableRow()
		: WeaponIndex(0)
		, Icon(nullptr)
		, SkeletalMesh(nullptr)
		, Damage(0)
		, AttackSpeed(0.0f)
		, Range(0.0f)
		, Price(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 WeaponIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price;
};

USTRUCT(BlueprintType)
struct FPotionTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FPotionTableRow()
		: PotionIndex(0)
		, Icon(nullptr)
		, StaticMesh(nullptr)
		, HealAmount(0)
		, Price(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 PotionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 HealAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price;
};

USTRUCT(BlueprintType)
struct FSkillBookTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FSkillBookTableRow()
		: SkillBookIndex(0)
		, Icon(nullptr)
		, StaticMesh(nullptr)
		, UpAmount(0)
		, Price(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 SkillBookIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UpAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price;
};

USTRUCT(BlueprintType)
struct FEnemyAbilityTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FEnemyAbilityTableRow()
		: EnemyIndex(0)
		, Icon(nullptr)
		, SkeletalMesh(nullptr)
		, EnemyMoney(0)
		, EnemyCurrentHp(0)
		, EnemyMaxHp(0)
		, EnemyAttackDamage(0)
		, EnemyDefence(0)
		, EnemyWeaponName("nullWeapon")
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 EnemyIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 EnemyMoney;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 EnemyCurrentHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 EnemyMaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 EnemyAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 EnemyDefence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FName EnemyWeaponName;
};

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	URGGameInstance();

protected:
	virtual void Init() override;

protected:
	UPROPERTY()
	class URGSaveGame* SaveGameInstance;

public:

	// �����Ϳ��� DataTable ������ �巡��&������� �Ҵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	UDataTable* WeaponDataTable;

	UPROPERTY()
	TArray<FName> WeaponRowNames;

	FWeaponTableRow* WeaponRow;

	/** DataTable FindRow �� ����� Context ���ڿ� */
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	FString ContextStr;

	// �����Ϳ��� DataTable ������ �巡��&������� �Ҵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	UDataTable* PotionDataTable;

	UPROPERTY()
	TArray<FName> PotionRowNames;

	FPotionTableRow* PotionRow;

	// �����Ϳ��� DataTable ������ �巡��&������� �Ҵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	UDataTable* SkillBookDataTable;

	UPROPERTY()
	TArray<FName> SkillBookRowNames;

	FSkillBookTableRow* SkillBookRow;

	// �����Ϳ��� DataTable ������ �巡��&������� �Ҵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	UDataTable* EnemyAbilityDataTable;

	UPROPERTY()
	TArray<FName> EnemyAbilityRowNames;

	FEnemyAbilityTableRow* EnemyAbilityRow;

	UPROPERTY()
	URGSaveGame* LoadGameInstance;

	//UPROPERTY()
	//URGSaveGame* CurrentSaveGame;


public:
	FWeaponTableRow* GetWeaponInformation(FName InWeaponName);

	FWeaponTableRow* GetRamdomWeapon();

	FName GetWeaponName(int32 InWeaponIndex);

	FPotionTableRow* GetPotionInformation(FName InPotionName);

	FPotionTableRow* GetRamdomPotion();

	FName GetPotionName(int32 InPotionIndex);

	FSkillBookTableRow* GetSkillBookInformation(FName InSkillBookName);

	FSkillBookTableRow* GetRamdomSkillBook();

	FName GetSkillBookName(int32 InSkillBookIndex);

	FEnemyAbilityTableRow* GetEnemyAbilityInformation(FName InEnemyName);

	// SaveGame ����
	void SetSaveGame(class URGPlayerDataAsset* NewSaveData);

	// SaveGame �ҷ�����
	class URGSaveGame* LoadSaveGameData();
};

