// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTTask_MoveToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RLCharacterEnemyDragon.h"

URLBTTask_MoveToTarget::URLBTTask_MoveToTarget()
{
	NodeName = TEXT("Move To Target Location");
	bNotifyTick = true;  // 매 틱마다 업데이트
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type URLBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	
	if (!BlackboardComp || !AIController)
	{
		return EBTNodeResult::Failed;
	}

	// 목표 위치 가져오기
	FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
	if (TargetLocation == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Target location is zero vector"));
		return EBTNodeResult::Failed;
	}

	// 드래곤 캐릭터 가져오기
	ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
	if (!Dragon)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Pawn is not a dragon"));
		return EBTNodeResult::Failed;
	}

	// 현재 위치에서 목표 위치까지의 거리 확인
	FVector CurrentLocation = Dragon->GetActorLocation();
	float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

	if (DistanceToTarget <= AcceptableRadius)
	{
		// 이미 목표 지점에 도착함
		UE_LOG(LogTemp, Log, TEXT("MoveToTarget: Already at target location"));
		return EBTNodeResult::Succeeded;
	}

	// Flying 모드 확인 및 설정
	UCharacterMovementComponent* MovementComp = Dragon->GetCharacterMovement();
	if (MovementComp && MovementComp->MovementMode != MOVE_Flying)
	{
		MovementComp->SetMovementMode(MOVE_Flying);
	}

	// AI 이동 명령 시작
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(TargetLocation, AcceptableRadius);
	
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: AI move request failed"));
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("MoveToTarget: Started moving to %s (Distance: %.2f)"), 
		   *TargetLocation.ToString(), DistanceToTarget);

	// InProgress 반환 - TickTask에서 계속 확인
	return EBTNodeResult::InProgress;
}

void URLBTTask_MoveToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	
	if (!BlackboardComp || !AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 목표 위치 가져오기
	FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
	
	// 드래곤 캐릭터 가져오기
	ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
	if (!Dragon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 현재 위치에서 목표 위치까지의 거리 확인
	FVector CurrentLocation = Dragon->GetActorLocation();
	float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

	// 목표 지점에 도달했는지 확인
	if (DistanceToTarget <= AcceptableRadius)
	{
		UE_LOG(LogTemp, Log, TEXT("MoveToTarget: Reached target location (Distance: %.2f)"), DistanceToTarget);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// AI 이동이 멈췄는지 확인
	if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		// 이동이 중단되었다면 다시 이동 시도
		EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(TargetLocation, AcceptableRadius);
		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Failed to restart movement"));
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}

EBTNodeResult::Type URLBTTask_MoveToTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}
	
	return EBTNodeResult::Aborted;
} 