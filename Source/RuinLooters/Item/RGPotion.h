// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/RGItemBoxBase.h"
#include "RGPotion.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARGPotion : public ARGItemBoxBase
{
	GENERATED_BODY()

public:
	ARGPotion();

protected:
	virtual void BeginPlay() override;

protected:
	struct FPotionTableRow* Potion;

public:
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
};
