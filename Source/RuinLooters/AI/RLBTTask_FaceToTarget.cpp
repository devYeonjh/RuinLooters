// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTTask_FaceToTarget.h"
#include "Character/RLCharacterPlayer.h"

EBTNodeResult::Type URLBTTask_FaceToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 블랙보드에서 Target 정보 가져오기
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	ARLCharacterPlayer* Target = Cast<ARLCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	if (Target)
	{
		// �÷��̾ �ٶ󺸵��� ��Ŀ�� ����
		AIController->SetFocus(Target);
	}
	else
	{
		// 타겟이 없을 때 포커스 해제
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	return EBTNodeResult::Succeeded;
}



