// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "RLEnemyAIController.generated.h"

class URLBTTask_Attack; // 전방 선언 추가

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API ARLEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    ARLEnemyAIController();

    // BTTask_Attack 인스턴스 등록/해제
    void RegisterAttackTask(URLBTTask_Attack* AttackTask);
    void UnregisterAttackTask(URLBTTask_Attack* AttackTask);
    
    // 등록된 BTTask_Attack 인스턴스 가져오기
    URLBTTask_Attack* GetAttackTask() const { return CurrentAttackTask; }

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

private:
    // 현재 등록된 공격 태스크
    UPROPERTY()
    URLBTTask_Attack* CurrentAttackTask;

public:
    void ShutdownAI();

};



