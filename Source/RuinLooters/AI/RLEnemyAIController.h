// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Engine/TimerHandle.h"
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

    // 블랙보드 키 상수들
    static const FName TargetKey;
    static const FName bInRangeKey;
    static const FName StartSkyPointKey;
    static const FName CurrentSkyPointKey;
    static const FName bIsHpLowKey;

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

    // 타이머 핸들
    FTimerHandle StartSkyPointTimerHandle;

    // 빙의된 후 호출
    virtual void OnPossess(APawn* InPawn) override;

    // 인식 변화시 호출
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim);

    // StartSkyPointKey 설정 함수
    UFUNCTION()
    void SetStartSkyPoint();

public:
    void ShutdownAI();

    // 새로운 BT 실행 함수들
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopCurrentBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void RestartCurrentBehaviorTree();
};



