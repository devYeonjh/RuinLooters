// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTTask_Attack.h"
#include "AIController.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Character/RLCharacterEnemy.h"

EBTNodeResult::Type URLBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 컨트롤러 포 찾기
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    ARLCharacterEnemy* Enemy = Cast<ARLCharacterEnemy>(Pawn);
    Enemy->Attack();

	return EBTNodeResult::Succeeded;
}



