// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "RGEnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARGEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    ARGEnemyAIController();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // 지각
    UPROPERTY(VisibleAnywhere)
    class UAIPerceptionComponent* Perception;

    // 시야
    UPROPERTY()
    class UAISenseConfig_Sight* SightCfg;

    UPROPERTY()
    class ARGCharacterPlayer* Player;

    // 빙의될 때 호출
    virtual void OnPossess(APawn* InPawn) override;

    // 지각 변화할 때 호출
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim);

public:
    void ShutdownAI();
};
