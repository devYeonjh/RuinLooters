// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RGBTService_FaceTarget.h"
#include "Character/RGCharacterPlayer.h"

URGBTService_FaceTarget::URGBTService_FaceTarget()
{
    NodeName = TEXT("Face Target");
    bNotifyBecomeRelevant = true;  // 서비스 시작 시에도 호출
    bNotifyTick = true;  // 매 Tick 호출
}

void URGBTService_FaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 블랙보드에서 Target 액터 가져오기
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	ARGCharacterPlayer* Target = Cast<ARGCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	if (Target)
	{
		// 플레이어를 바라보도록 포커스 설정
		AIController->SetFocus(Target);
	}
	else
	{
		// 타겟이 없을 땐 포커스 해제
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

}
