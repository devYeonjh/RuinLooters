// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "RGBTService_FaceTarget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGBTService_FaceTarget : public UBTService
{
	GENERATED_BODY()

public:
    URGBTService_FaceTarget();

protected:
    // 플레이어가 저장된 블랙보드 키
    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetKey;

    // 매 틱 실행
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
