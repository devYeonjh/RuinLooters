// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "RLBTService_FaceTarget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLBTService_FaceTarget : public UBTService
{
	GENERATED_BODY()

public:
    URLBTService_FaceTarget();

protected:
    // �÷��̾ �����?�������� Ű
    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetKey;

    // 매 틱 실행
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};



