// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTTask_Attack.h"
#include "AIController.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Character/RLCharacterEnemy.h"
#include "Character/RLCharacterAttackInterface.h"
#include "AI/RLEnemyAIController.h"

URLBTTask_Attack::URLBTTask_Attack()
{
    // 필요시 초기화
}

URLBTTask_Attack::~URLBTTask_Attack()
{
    // 소멸자에서 AI Controller에서 등록 해제
}

EBTNodeResult::Type URLBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI Controller에 자신을 등록
    ARLEnemyAIController* EnemyAI = Cast<ARLEnemyAIController>(OwnerComp.GetAIOwner());
    if (EnemyAI)
    {
        EnemyAI->RegisterAttackTask(this);
    }
    
    // 컨트롤러 폰 찾기
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    IRLCharacterAttackInterface* Enemy = Cast<IRLCharacterAttackInterface>(Pawn);
    if (Enemy)
    {
        // BehaviorTreeComponent 저장
        CachedOwnerComp = &OwnerComp;
        
        // 공격 시작 (바인딩은 캐릭터 BeginPlay에서 이미 완료)
        Enemy->Attack();
        
        // 공격이 시작되었으므로 InProgress 반환
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

EBTNodeResult::Type URLBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI Controller에서 등록 해제
    ARLEnemyAIController* EnemyAI = Cast<ARLEnemyAIController>(OwnerComp.GetAIOwner());
    if (EnemyAI)
    {
        EnemyAI->UnregisterAttackTask(this);
    }
    
    CachedOwnerComp = nullptr;
    return EBTNodeResult::Aborted;
}

void URLBTTask_Attack::OnAttackCompleted()
{
    if (CachedOwnerComp.IsValid())
    {
        // 태스크 완료 처리
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
        CachedOwnerComp = nullptr;
    }
}
