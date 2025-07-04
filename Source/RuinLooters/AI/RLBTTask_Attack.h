// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RLBTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	// Task가 시작될 때 호출
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};



