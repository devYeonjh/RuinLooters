// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RLBTTask_SetRandomSkyPoint.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_SetRandomSkyPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
    URLBTTask_SetRandomSkyPoint();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
}; 