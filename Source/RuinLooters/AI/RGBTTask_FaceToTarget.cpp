// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RGBTTask_FaceToTarget.h"
#include "Character/RGCharacterPlayer.h"

EBTNodeResult::Type URGBTTask_FaceToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 블랙보드에서 Target 액터 가져오기
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	ARGCharacterPlayer* Target = Cast<ARGCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

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

	return EBTNodeResult::Succeeded;
}
