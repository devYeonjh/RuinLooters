// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/RLItemBoxBase.h"
#include "RLWeapon.generated.h"

UCLASS(Abstract)
class RUINLOOTERS_API ARLWeapon : public ARLItemBoxBase
{
	GENERATED_BODY()

public:
	ARLWeapon();

protected:
	virtual void BeginPlay() override;

	struct FWeaponTableRow* DroppedWeaponRow;

public:
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};



