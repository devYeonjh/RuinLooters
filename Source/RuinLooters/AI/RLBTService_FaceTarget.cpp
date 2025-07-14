// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RLBTService_FaceTarget.h"
#include "Character/RLCharacterPlayer.h"

URLBTService_FaceTarget::URLBTService_FaceTarget()
{
    NodeName = TEXT("Face Target");
    bNotifyBecomeRelevant = true;  // 서비스 노드 중요시 호출
    bNotifyTick = true;  // 매 Tick 호출
}

void URLBTService_FaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 블랙보드에서 Target 정보 가져오기
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
		// 타겟이 없을 때 포커스 해제
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

}



