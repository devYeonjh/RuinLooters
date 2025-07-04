// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "RLBTService_CheckDistance.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLBTService_CheckDistance : public UBTService
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector bInRangeKey;

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};



