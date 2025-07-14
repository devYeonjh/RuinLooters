// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPWidget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API UHPWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	int32 StatCurrentHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	int32 StatMaxHp;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HpBar;

public:
	void CalculateHp(int32 NewCurrentHp, int32 NewMaxHp);
	void DestroyWidget();
};



