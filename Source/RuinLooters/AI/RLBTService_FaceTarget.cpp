// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTService_FaceTarget.h"
#include "Character/RLCharacterPlayer.h"

URLBTService_FaceTarget::URLBTService_FaceTarget()
{
    NodeName = TEXT("Face Target");
    bNotifyBecomeRelevant = true;  // ���� ���� �ÿ��� ȣ��
    bNotifyTick = true;  // �� Tick ȣ��
}

void URLBTService_FaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// �������忡�� Target ���� ��������
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	ARLCharacterPlayer* Target = Cast<ARLCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

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

}



