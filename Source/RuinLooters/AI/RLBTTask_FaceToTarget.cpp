// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTTask_FaceToTarget.h"
#include "Character/RLCharacterPlayer.h"

EBTNodeResult::Type URLBTTask_FaceToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// �������忡�� Target ���� ��������
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
		// Ÿ���� ���� �� ��Ŀ�� ����
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	return EBTNodeResult::Succeeded;
}



