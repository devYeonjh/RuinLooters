// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RLBTTask_BreathAttack.generated.h"

/**
 * 드래곤의 브레스 어택을 실행하는 태스크
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_BreathAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	URLBTTask_BreathAttack();

protected:
	// Task가 시작될 때 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
}; 