// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameInstance/RGGameInstance.h"
#include "RGSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	URGSaveGame()
		: SaveSlotName(TEXT("nullName"))
		, PlayerIndex(0)
		, PlayerMoney(0)
		, CurrentHp(0)
		, MaxHp(0)
		, AttackDamage(0)
		, Defence(0)
		, SaveWeaponName(TEXT("nullWeapon"))
		, StageIndex(0)
	{
	}

	void SetSaveGameData(class URGPlayerDataAsset* NewSaveData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 PlayerIndex;


	// Player ---------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 PlayerMoney;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 CurrentHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 Defence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FName SaveWeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 StageIndex;

};
