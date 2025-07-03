// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/RGItemBoxBase.h"
#include "RGWeapon.generated.h"

UCLASS(Abstract)
class RUINLOOTERS_API ARGWeapon : public ARGItemBoxBase
{
	GENERATED_BODY()

public:
	ARGWeapon();

protected:
	virtual void BeginPlay() override;

	struct FWeaponTableRow* DroppedWeaponRow;

public:
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
