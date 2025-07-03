// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/RLItemBoxBase.h"
#include "RLPotion.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLPotion : public ARLItemBoxBase
{
	GENERATED_BODY()

public:
	ARLPotion();

protected:
	virtual void BeginPlay() override;

protected:
	struct FPotionTableRow* Potion;

public:
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
};



