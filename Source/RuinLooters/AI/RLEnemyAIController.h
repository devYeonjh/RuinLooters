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

    // ����
    UPROPERTY(VisibleAnywhere)
    class UAIPerceptionComponent* Perception;

    // �þ�
    UPROPERTY()
    class UAISenseConfig_Sight* SightCfg;

    UPROPERTY()
    class ARLCharacterPlayer* Player;

    // ���ǵ� �� ȣ��
    virtual void OnPossess(APawn* InPawn) override;

    // ���� ��ȭ�� �� ȣ��
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim);

public:
    void ShutdownAI();
};



