// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RLBTTask_FlyDirectly.generated.h"

/**
 * 드래곤이 하늘에서 목표 지점으로 직선 이동하는 태스크
 * 3D 공간에서 정확한 직선 이동을 보장합니다.
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_FlyDirectly : public UBTTaskNode
{
	GENERATED_BODY()

public:
	URLBTTask_FlyDirectly();

protected:
	// 허용 반경 (이 거리 내에 도달하면 성공)
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptableRadius = 150.0f;

	// 이동 속도
	UPROPERTY(EditAnywhere, Category = "AI")
	float MovementSpeed = 1000.0f;

	// 회전 속도 (도/초)
	UPROPERTY(EditAnywhere, Category = "AI")
	float RotationSpeed = 180.0f;

	// 목표 방향으로 회전할지 여부
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bRotateTowardsTarget = true;

	// 최대 이동 시간 (초) - 이 시간이 지나면 실패
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxMoveTime = 7.0f;

	// Task 시작
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Task 진행 상황 업데이트
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 경과 시간 추적
	float ElapsedTime;

	// 목표 위치 캐싱
	FVector CachedTargetLocation;

	// 시작 위치 캐싱
	FVector StartLocation;

	// 이동 거리 캐싱
	float TotalDistance;
}; 