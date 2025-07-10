// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTTask_BreathAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Character/RLCharacterEnemyDragon.h"

URLBTTask_BreathAttack::URLBTTask_BreathAttack()
{
    // 태스크 이름 설정
    NodeName = TEXT("Dragon Breath Attack");
    
    // 태스크가 즉시 완료되도록 설정
    bNotifyTick = false;
    bNotifyTaskFinished = false;
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
    
    // 태스크 성공 반환
    return EBTNodeResult::Succeeded;
} 