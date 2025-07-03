// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/RLItemBoxBase.h"
#include "RLSkillBook.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLSkillBook : public ARLItemBoxBase
{
	GENERATED_BODY()
	
public:
	ARLSkillBook();

protected:
	virtual void BeginPlay() override;

protected:
	struct FSkillBookTableRow* SkillBook;


public:
	UFUNCTION()
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};



