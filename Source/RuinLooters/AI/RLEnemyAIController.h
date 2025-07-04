// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "RLEnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    ARLEnemyAIController();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // 인식
    UPROPERTY(VisibleAnywhere)
    class UAIPerceptionComponent* Perception;

    // 시야
    UPROPERTY()
    class UAISenseConfig_Sight* SightCfg;

    UPROPERTY()
    class ARLCharacterPlayer* Player;

    // 빙의된 후 호출
    virtual void OnPossess(APawn* InPawn) override;

    // 인식 변화시 호출
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim);

public:
    void ShutdownAI();
};



