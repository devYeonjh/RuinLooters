// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTTask_LandToGround.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RLCharacterEnemyDragon.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "AI/RLEnemyAIController.h"

URLBTTask_LandToGround::URLBTTask_LandToGround()
{
    // 태스크 이름 설정
    NodeName = TEXT("Land To Ground");
    
    // 태스크가 틱을 받을 수 있도록 설정
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    
    // 기본값 초기화
    AcceptableGroundDistance = 50.0f;
    ForceDownSpeed = 500.0f;
    GroundCheckDistance = 2000.0f;
    MaxWaitTime = 10.0f;
    GravityCheckTime = 1.0f;
    
    // BT 교체 관련 변수 초기화
    NewBehaviorTreeOnLanding = nullptr;
    bSwitchBehaviorTreeOnLanding = false;
    
    // 내부 변수 초기화
    ElapsedTime = 0.0f;
    GravityCheckElapsed = 0.0f;
    StartLocation = FVector::ZeroVector;
    PreviousLocation = FVector::ZeroVector;
    bForceDownMode = false;
}

EBTNodeResult::Type URLBTTask_LandToGround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI 컨트롤러 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: AI Controller is null"));
        return EBTNodeResult::Failed;
    }
    
    // 캐릭터 가져오기
    ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: Character is null"));
        return EBTNodeResult::Failed;
    }
    
    // 캐릭터 무브먼트 컴포넌트 가져오기
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: Movement component is null"));
        return EBTNodeResult::Failed;
    }
    
    // MovementMode를 Walk로 변경
    MovementComp->SetMovementMode(MOVE_Walking);
    
    // 시작 위치 설정
    StartLocation = Character->GetActorLocation();
    PreviousLocation = StartLocation;
    
    // 경과 시간 초기화
    ElapsedTime = 0.0f;
    GravityCheckElapsed = 0.0f;
    bForceDownMode = false;
    
    // 이미 땅에 가까이 있는지 확인
    float DistanceToGround = GetDistanceToGround(Character);
    if (DistanceToGround <= AcceptableGroundDistance)
    {
        UE_LOG(LogTemp, Log, TEXT("LandToGround Task: Already close to ground (Distance: %.1f)"), DistanceToGround);
        return EBTNodeResult::Succeeded;
    }
    
    UE_LOG(LogTemp, Log, TEXT("LandToGround Task: Starting landing process. Movement mode set to Walking. Distance to ground: %.1f"), DistanceToGround);
    
    return EBTNodeResult::InProgress;
}

void URLBTTask_LandToGround::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 경과 시간 업데이트
    ElapsedTime += DeltaSeconds;
    GravityCheckElapsed += DeltaSeconds;
    
    // 최대 시간 초과 검사
    if (ElapsedTime >= MaxWaitTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: Max wait time exceeded (%.1f seconds)"), ElapsedTime);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // AI 컨트롤러와 캐릭터 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
    if (!Character)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 현재 위치 가져오기
    FVector CurrentLocation = Character->GetActorLocation();
    
    // 땅까지의 거리 계산
    float DistanceToGround = GetDistanceToGround(Character);
    
    // 땅에 도달했는지 확인
    if (DistanceToGround <= AcceptableGroundDistance)
    {
        UE_LOG(LogTemp, Log, TEXT("LandToGround Task: Successfully landed! Distance to ground: %.1f"), DistanceToGround);
        
        // 착지 후 BT 교체 처리
        HandleBehaviorTreeSwitch(OwnerComp);
        
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    
    // 중력이 작동하는지 확인 (일정 시간 동안 위치 변화 확인)
    if (GravityCheckElapsed >= GravityCheckTime)
    {
        float VerticalMovement = PreviousLocation.Z - CurrentLocation.Z;
        
        // 중력이 작동하지 않는다면 (거의 움직이지 않았다면)
        if (FMath::Abs(VerticalMovement) < 10.0f && !bForceDownMode)
        {
            bForceDownMode = true;
            UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: Gravity not working properly, switching to force down mode"));
        }
        
        // 위치 업데이트
        PreviousLocation = CurrentLocation;
        GravityCheckElapsed = 0.0f;
    }
    
    // 강제 하강 모드인 경우
    if (bForceDownMode)
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            // 아래쪽으로 강제 이동
            FVector DownwardVelocity = FVector(0.0f, 0.0f, -ForceDownSpeed);
            MovementComp->AddInputVector(FVector::DownVector, 1.0f);
            
            // 직접 위치 조정 (더 확실한 방법)
            FVector NewLocation = CurrentLocation + (FVector::DownVector * ForceDownSpeed * DeltaSeconds);
            Character->SetActorLocation(NewLocation);
            
            UE_LOG(LogTemp, Log, TEXT("LandToGround Task: Force moving down. Current Z: %.1f, Target Z adjustment: %.1f"), 
                   CurrentLocation.Z, -ForceDownSpeed * DeltaSeconds);
        }
    }
    
    // 디버그 로그
    if (FMath::Fmod(ElapsedTime, 1.0f) < DeltaSeconds) // 1초마다 로그
    {
        UE_LOG(LogTemp, Log, TEXT("LandToGround Task: Distance to ground: %.1f, Force down mode: %s"), 
               DistanceToGround, bForceDownMode ? TEXT("True") : TEXT("False"));
    }
}

float URLBTTask_LandToGround::GetDistanceToGround(ACharacter* Character)
{
    if (!Character || !Character->GetWorld())
    {
        return -1.0f;
    }
    
    // 캐릭터 위치에서 아래쪽으로 라인 트레이스
    FVector TraceStartLocation = Character->GetActorLocation();
    FVector EndLocation = TraceStartLocation - FVector(0.0f, 0.0f, GroundCheckDistance);
    
    // 충돌 쿼리 파라미터 설정
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Character);
    CollisionParams.bTraceComplex = false;
    CollisionParams.bReturnPhysicalMaterial = false;
    
    // 라인 트레이스 실행
    FHitResult HitResult;
    bool bHit = Character->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        CollisionParams
    );
    
    if (bHit)
    {
        float Distance = FVector::Dist(TraceStartLocation, HitResult.Location);
        return Distance;
    }
    
    // 땅을 찾지 못한 경우 최대 거리 반환
    return GroundCheckDistance;
} 

void URLBTTask_LandToGround::HandleBehaviorTreeSwitch(UBehaviorTreeComponent& OwnerComp)
{
    // BT 교체 기능이 비활성화되어 있으면 리턴
    if (!bSwitchBehaviorTreeOnLanding)
    {
        UE_LOG(LogTemp, Log, TEXT("LandToGround Task: BT switch is disabled"));
        return;
    }
    
    // 새로운 BT 에셋이 설정되지 않았으면 리턴
    if (!NewBehaviorTreeOnLanding)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: NewBehaviorTreeOnLanding is not set"));
        return;
    }
    
    // AI 컨트롤러 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: AI Controller is null"));
        return;
    }
    
    // RLEnemyAIController로 캐스팅
    ARLEnemyAIController* EnemyAIController = Cast<ARLEnemyAIController>(AIController);
    if (!EnemyAIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandToGround Task: AI Controller is not RLEnemyAIController"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("LandToGround Task: Switching to new behavior tree on landing"));
    
    // 새로운 BT로 교체
    EnemyAIController->SwitchBehaviorTree(NewBehaviorTreeOnLanding);
} 