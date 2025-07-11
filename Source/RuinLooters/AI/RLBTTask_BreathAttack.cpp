// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTTask_BreathAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Character/RLCharacterEnemyDragon.h"
#include "Engine/Engine.h"

URLBTTask_BreathAttack::URLBTTask_BreathAttack()
{
    // 태스크 이름 설정
    NodeName = TEXT("Dragon Breath Attack");
    
    // 태스크가 틱을 받을 수 있도록 설정
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    
    // 3초 지연 시간 설정
    DelayTime = 3.0f;
    ElapsedTime = 0.0f;
}

EBTNodeResult::Type URLBTTask_BreathAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI 컨트롤러 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("BreathAttack Task: AI Controller is null"));
        return EBTNodeResult::Failed;
    }
    
    // 드래곤 캐릭터 가져오기
    ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
    if (!Dragon)
    {
        UE_LOG(LogTemp, Warning, TEXT("BreathAttack Task: Dragon character is null"));
        return EBTNodeResult::Failed;
    }
    
    // 브레스 어택 실행
    Dragon->BreathAttack();
    
    // 성공적으로 실행됨을 로그
    UE_LOG(LogTemp, Log, TEXT("Dragon Breath Attack executed successfully"));
    
    // 경과 시간 초기화
    ElapsedTime = 0.0f;
    
    // InProgress 상태 반환 (3초 후 Succeeded 반환)
    return EBTNodeResult::InProgress;
}

void URLBTTask_BreathAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 경과 시간 누적
    ElapsedTime += DeltaSeconds;
    
    // 3초가 지나면 태스크 완료
    if (ElapsedTime >= DelayTime)
    {
        UE_LOG(LogTemp, Log, TEXT("Dragon Breath Attack completed after %.2f seconds"), ElapsedTime);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
} 