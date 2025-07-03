// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTService_CheckDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Character/RLCharacterPlayer.h"

void URLBTService_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard) return;

    ARLCharacterPlayer* Target = Cast<ARLCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
    APawn* SelfPawn = OwnerComp.GetAIOwner()->GetPawn();

    bool bInRange = false;

    if (Target && SelfPawn)
    {
        const float Distance = FVector::Dist(SelfPawn->GetActorLocation(), Target->GetActorLocation());
        // �Ÿ��� ��Ÿ�����?ª���� true
        bInRange = (Distance <= AttackRange);
    }

    // �Ÿ��� ���� bInRange�� �����?��ȯ
    Blackboard->SetValueAsBool(InRangeKey.SelectedKeyName, bInRange);
}



