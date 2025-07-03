// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/RGItemBoxBase.h"
#include "RGSkillBook.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARGSkillBook : public ARGItemBoxBase
{
	GENERATED_BODY()
	
public:
	ARGSkillBook();

protected:
	virtual void BeginPlay() override;

protected:
	struct FSkillBookTableRow* SkillBook;


public:
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
