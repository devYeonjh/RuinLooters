// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "RLBTService_CheckHp.generated.h"

/**
 * 드래곤의 체력을 체크하여 체력이 50% 이하일 때 bIsHpLowKey를 true로 설정하는 서비스
 */
UCLASS()
class RUINLOOTERS_API URLBTService_CheckHp : public UBTService
{
	GENERATED_BODY()
	
public:
	URLBTService_CheckHp();

protected:
	// 체력 체크 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector bIsHpLowKey;

	// 체력 임계값 비율 (기본값: 0.5f = 50%)
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HpThreshold = 0.5f;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
}; 