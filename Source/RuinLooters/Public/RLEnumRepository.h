// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	PT_Sword = 0 UMETA(DisplayName = "Sword"),
	PT_Gun = 1 UMETA(DisplayName = "Gun"),
};

/**
 * 
 */
class RUINLOOTERS_API RLEnumRepository
{
public:
	RLEnumRepository();
};



