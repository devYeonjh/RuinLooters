// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTService_CheckDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Character/RLCharacterPlayer.h"
#include "AI/RLEnemyAIController.h"

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
        // 거리가 어택랜지보다 짧으면 true
        bInRange = (Distance <= AttackRange);
    }

    // 거리에 따른 bInRange를 블랙보드에 반환
    Blackboard->SetValueAsBool(bInRangeKey.SelectedKeyName, bInRange);

    // 랜덤 bool 값 생성 (50% 확률로 true/false)
    bool bRandomValue = FMath::RandBool();
    
    // bRandomKey를 블랙보드에 설정
    Blackboard->SetValueAsBool(ARLEnemyAIController::bRandomKey, bRandomValue);
    
    UE_LOG(LogTemp, Log, TEXT("Random bool value set: %s"), bRandomValue ? TEXT("True") : TEXT("False"));
}



