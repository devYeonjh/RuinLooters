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

	// Task 진행 중 매 틱마다 호출
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 지연 시간 (초)
	float DelayTime;
	
	// 경과 시간 (초)
	float ElapsedTime;
}; 