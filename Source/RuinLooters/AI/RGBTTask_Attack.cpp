// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RGBTTask_Attack.h"
#include "AIController.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Character/RGCharacterEnemy.h"

EBTNodeResult::Type URGBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 본인의 폰 찾기
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    ARGCharacterEnemy* Enemy = Cast<ARGCharacterEnemy>(Pawn);
    Enemy->Attack();

	return EBTNodeResult::Succeeded;
}
