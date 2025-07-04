// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "RLBTTask_FaceToTarget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_FaceToTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
	// �÷��̾ �����?�������� Ű
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetKey;

protected:
	// Task�� ���۵� �� ȣ��
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};



