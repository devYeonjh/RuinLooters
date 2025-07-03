// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RGBTService_CheckDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Character/RGCharacterPlayer.h"

void URGBTService_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return;

    ARGCharacterPlayer* Target = Cast<ARGCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
    APawn* SelfPawn = OwnerComp.GetAIOwner()->GetPawn();

    bool bInRange = false;

    if (Target && SelfPawn)
    {
        const float Distance = FVector::Dist(SelfPawn->GetActorLocation(), Target->GetActorLocation());
        // 거리가 사거리보다 짧으면 true
        bInRange = (Distance <= AttackRange);
    }

    // 거리에 따른 bInRange의 결과값 반환
    Blackboard->SetValueAsBool(InRangeKey.SelectedKeyName, bInRange);
}
