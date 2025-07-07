// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTTask_Attack.h"
#include "AIController.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Character/RLCharacterEnemy.h"
#include "Character/RLCharacterAttackInterface.h"
#include "AI/RLEnemyAIController.h"

EBTNodeResult::Type URLBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 컨트롤러 폰 찾기
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    IRLCharacterAttackInterface* Enemy = Cast<IRLCharacterAttackInterface>(Pawn);
    if (Enemy)
    {
        // 공격 시작
        Enemy->Attack();
        
        // 공격이 시작되었으므로 Succeeded 반환
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
