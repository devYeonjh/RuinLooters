// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RLBTTask_MoveToTarget.generated.h"

/**
 * 드래곤 전용 이동 태스크
 * 목표 지점에 정확히 도달할 때까지 InProgress 상태를 유지합니다.
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_MoveToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	URLBTTask_MoveToTarget();

protected:
	// 목표 위치 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetLocationKey;

	// 허용 반경 (이 거리 내에 도달하면 성공)
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptableRadius = 100.0f;

	// 이동 속도
	UPROPERTY(EditAnywhere, Category = "AI")
	float MovementSpeed = 600.0f;

	// Task 시작
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Task 진행 상황 업데이트
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Task 중단
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
}; 