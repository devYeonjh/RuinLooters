// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTTask_SetRandomSkyPoint.h"
#include "AI/RLEnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Math/UnrealMathUtility.h"

URLBTTask_SetRandomSkyPoint::URLBTTask_SetRandomSkyPoint()
{
    NodeName = TEXT("Set Random Sky Point");
}

EBTNodeResult::Type URLBTTask_SetRandomSkyPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    // StartSkyPointKey에서 기준점 가져오기
    FVector StartSkyPoint = BlackboardComp->GetValueAsVector(ARLEnemyAIController::StartSkyPointKey);
    
    // -1000 ~ 1000 범위에서 랜덤한 X, Y 오프셋 생성
    float RandomX = FMath::RandRange(-1500.0f, 1500.0f);
    float RandomY = FMath::RandRange(-1500.0f, 1500.0f);
    float RandomZ = FMath::RandRange(-500.0f, 500.0f);
    
    // 새로운 CurrentSkyPoint 계산 (Z값은 그대로 유지)
    FVector CurrentSkyPoint = FVector(
        StartSkyPoint.X + RandomX,
        StartSkyPoint.Y + RandomY,
        StartSkyPoint.Z + RandomZ
    );
    
    // CurrentSkyPointKey에 새로운 위치 설정
    BlackboardComp->SetValueAsVector(ARLEnemyAIController::CurrentSkyPointKey, CurrentSkyPoint);
    
    UE_LOG(LogTemp, Warning, TEXT("Random Sky Point set to: %s (Offset: X=%.2f, Y=%.2f)"), 
           *CurrentSkyPoint.ToString(), RandomX, RandomY);
    
    return EBTNodeResult::Succeeded;
} 