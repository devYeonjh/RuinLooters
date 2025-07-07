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

public:
	// 생성자/소멸자 추가
	URLBTTask_Attack();
	virtual ~URLBTTask_Attack();
	
	// 공격 완료 콜백 함수 (public으로 이동)
	UFUNCTION()
	void OnAttackCompleted();

protected:
	// Task가 시작될 때 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// Task가 중단될 때 호출
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 델리게이트 핸들을 저장할 변수
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};



