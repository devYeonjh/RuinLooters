// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGame/RGSaveGame.h"
#include "Character/RGPlayerDataAsset.h"

void URGSaveGame::SetSaveGameData(URGPlayerDataAsset* NewSaveData)
{
	SaveSlotName = NewSaveData->SaveSlotName;
	PlayerIndex = NewSaveData->PlayerIndex;
	PlayerMoney = NewSaveData->PlayerMoney;
	MaxHp = NewSaveData->MaxHp;
	CurrentHp = NewSaveData->CurrentHp;
	AttackDamage = NewSaveData->AttackDamage;
	Defence = NewSaveData->Defence;
	SaveWeaponName = NewSaveData->SaveWeaponName;
	StageIndex = NewSaveData->StageIndex;
}
