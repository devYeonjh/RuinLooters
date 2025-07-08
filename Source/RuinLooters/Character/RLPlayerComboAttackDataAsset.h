// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RLPlayerComboAttackDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLPlayerComboAttackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	URLPlayerComboAttackDataAsset();

	UPROPERTY(EditAnywhere, Category = Name)
	FString MontageSectionNamePrefix;

	UPROPERTY(EditAnywhere, Category = Name)
	uint8 MaxComboCount;

	UPROPERTY(EditAnywhere, Category = Name)
	float FrameRate;

	UPROPERTY(EditAnywhere, Category = ComboData)
	TArray<float> EffectiveFrameCount;
};
