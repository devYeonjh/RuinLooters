// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RLNPC.h"
#include "Character/RLNPCBuyInterface.h"
#include "RLSkillBookNPC.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLSkillBookNPC : public ARLNPC, public IRLNPCBuyInterface
{
	GENERATED_BODY()

protected:
	struct FSkillBookTableRow* SkillBook;
	
protected:
	virtual void OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void HandleStoreBuy() override;
};



