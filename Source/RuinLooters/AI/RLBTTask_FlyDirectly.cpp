// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTTask_FlyDirectly.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RLCharacterEnemyDragon.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

URLBTTask_FlyDirectly::URLBTTask_FlyDirectly()
{
    // 태스크 이름 설정
    NodeName = TEXT("Fly Directly");
    
    // 태스크가 틱을 받을 수 있도록 설정
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    
    // 기본값 초기화
    AcceptableRadius = 150.0f;
    MovementSpeed = 1000.0f;
    RotationSpeed = 180.0f;
    bRotateTowardsTarget = true;
    MaxMoveTime = 7.0f;
    
    // 내부 변수 초기화
    ElapsedTime = 0.0f;
    CachedTargetLocation = FVector::ZeroVector;
    StartLocation = FVector::ZeroVector;
    TotalDistance = 0.0f;
}

EBTNodeResult::Type URLBTTask_FlyDirectly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI 컨트롤러 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyDirectly Task: AI Controller is null"));
        return EBTNodeResult::Failed;
    }
    
    // 드래곤 캐릭터 가져오기
    ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
    if (!Dragon)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyDirectly Task: Dragon character is null"));
        return EBTNodeResult::Failed;
    }
    
    // 블랙보드 컴포넌트 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyDirectly Task: Blackboard component is null"));
        return EBTNodeResult::Failed;
    }
    
    // 목표 위치 가져오기 (CurrentSkyPointKey 사용)
    FVector TargetLocation = BlackboardComp->GetValueAsVector(TEXT("CurrentSkyPointKey"));
    if (TargetLocation.IsZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyDirectly Task: CurrentSkyPointKey is zero or invalid"));
        return EBTNodeResult::Failed;
    }
    
    // 시작 위치 설정
    StartLocation = Dragon->GetActorLocation();
    CachedTargetLocation = TargetLocation;
    TotalDistance = FVector::Dist(StartLocation, CachedTargetLocation);
    
    // 목표가 너무 가까우면 즉시 성공
    if (TotalDistance <= AcceptableRadius)
    {
        UE_LOG(LogTemp, Log, TEXT("FlyDirectly Task: Already at target location"));
        return EBTNodeResult::Succeeded;
    }
    
    // 드래곤을 비행 모드로 설정
    UCharacterMovementComponent* MovementComp = Dragon->GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->SetMovementMode(MOVE_Flying);
        MovementComp->MaxFlySpeed = MovementSpeed;
        
        // 회전 속도 설정
        MovementComp->RotationRate = FRotator(0.0f, RotationSpeed, 0.0f);
        
        UE_LOG(LogTemp, Log, TEXT("FlyDirectly Task: Set movement mode to flying, speed: %.1f"), MovementSpeed);
    }
    
    // 경과 시간 초기화
    ElapsedTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("FlyDirectly Task: Starting flight from %s to %s (Distance: %.1f)"), 
           *StartLocation.ToString(), *CachedTargetLocation.ToString(), TotalDistance);
    
    return EBTNodeResult::InProgress;
}

void URLBTTask_FlyDirectly::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 경과 시간 업데이트
    ElapsedTime += DeltaSeconds;
    
    // 최대 시간 초과 검사
    if (ElapsedTime >= MaxMoveTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlyDirectly Task: Max move time exceeded (%.1f seconds)"), ElapsedTime);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // AI 컨트롤러와 드래곤 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
    if (!Dragon)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 현재 위치와 목표까지의 거리 계산
    FVector CurrentLocation = Dragon->GetActorLocation();
    float DistanceToTarget = FVector::Dist(CurrentLocation, CachedTargetLocation);
    
    // 목표에 도달했는지 확인
    if (DistanceToTarget <= AcceptableRadius)
    {
        UE_LOG(LogTemp, Log, TEXT("FlyDirectly Task: Reached target! Distance: %.1f"), DistanceToTarget);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    
    // 이동 방향 계산
    FVector DirectionToTarget = (CachedTargetLocation - CurrentLocation).GetSafeNormal();
    
    // 캐릭터 무브먼트 컴포넌트 가져오기
    UCharacterMovementComponent* MovementComp = Dragon->GetCharacterMovement();
    if (!MovementComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 직선 이동 입력 적용
    MovementComp->AddInputVector(DirectionToTarget, 1.0f);
    
    // 목표 방향으로 회전 (옵션)
    if (bRotateTowardsTarget)
    {
        FRotator TargetRotation = DirectionToTarget.Rotation();
        FRotator CurrentRotation = Dragon->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed / 180.0f);
        Dragon->SetActorRotation(NewRotation);
    }
    
}

