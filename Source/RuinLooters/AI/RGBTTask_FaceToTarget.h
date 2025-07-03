// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "RGBTTask_FaceToTarget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGBTTask_FaceToTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
	// 플레이어가 저장된 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetKey;

protected:
	// Task가 시작될 때 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
