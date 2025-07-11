// EnemyAIController.cpp
#include "AI/RLEnemyAIController.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

// 블랙보드 키 상수들 정의
const FName ARLEnemyAIController::TargetKey(TEXT("TargetKey"));
const FName ARLEnemyAIController::bInRangeKey(TEXT("bInRangeKey"));
const FName ARLEnemyAIController::StartSkyPointKey(TEXT("StartSkyPointKey"));
const FName ARLEnemyAIController::CurrentSkyPointKey(TEXT("CurrentSkyPointKey"));
const FName ARLEnemyAIController::bIsHpLowKey(TEXT("bIsHpLowKey"));
const FName ARLEnemyAIController::bRandomKey(TEXT("bRandomKey"));

ARLEnemyAIController::ARLEnemyAIController()
{
    Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
    SightCfg = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
    SightCfg->SightRadius = 10000.f;
    SightCfg->LoseSightRadius = 10300.f;
    SightCfg->PeripheralVisionAngleDegrees = 180.f;
    Perception->ConfigureSense(*SightCfg);
    Perception->SetDominantSense(SightCfg->GetSenseImplementation());

    SightCfg->DetectionByAffiliation.bDetectEnemies = true;   // 다른 팀
    SightCfg->DetectionByAffiliation.bDetectFriendlies = false;  // 같은 팀 감지
    SightCfg->DetectionByAffiliation.bDetectNeutrals = false;  // 중립 감지

    Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ARLEnemyAIController::OnPerceptionUpdated);
}

void ARLEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 필요한 플레이어 찾기
    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    // 임시 raw 포인터 준비
    UBlackboardComponent* BlackboardComp = nullptr;

    // 자동으로 블랙보드가 설정
    if (UseBlackboard(BlackboardAsset, BlackboardComp))
    {
        // TObjectPtr 대신에 Blackboard 를 설정
        Blackboard = BlackboardComp;

        // 현재 AAIController::Blackboard 대신에 가져온 블랙보드 트리 시작
        RunBehaviorTree(BehaviorTreeAsset);
        UE_LOG(LogTemp, Warning, TEXT("Controller OnPossess"));

        // 1초 후 StartSkyPointKey 설정
        GetWorld()->GetTimerManager().SetTimer(StartSkyPointTimerHandle, this, &ARLEnemyAIController::SetStartSkyPoint, 1.0f, false);
    }
}

void ARLEnemyAIController::SetStartSkyPoint()
{
    if (!Blackboard) return;

    // 현재 폰의 위치를 StartSkyPointKey로 설정
    if (APawn* ControlledPawn = GetPawn())
    {
        FVector StartSkyPoint = ControlledPawn->GetActorLocation();
        Blackboard->SetValueAsVector(StartSkyPointKey, StartSkyPoint);
        UE_LOG(LogTemp, Warning, TEXT("StartSkyPointKey set to: %s"), *StartSkyPoint.ToString());
    }
}

void ARLEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim)
{
    UE_LOG(LogTemp, Warning, TEXT("OnPerceptionUpdated"));
    if (!Blackboard) return;

    Player = Cast<ARLCharacterPlayer>(Actor);
    if (Player)
    {
        // Stim: 자극에 대한 정보를 담은 객체
        if (Stim.WasSuccessfullySensed())
        {
            // "TargetKey"를 키로 타겟 설정
            Blackboard->SetValueAsObject(TargetKey, Player);
            UE_LOG(LogTemp, Warning, TEXT("Enemy Detected Player"));
        }
        // 시야에서 사라졌다면
        else
        {
            Player = Cast<ARLCharacterPlayer>(Blackboard->GetValueAsObject(TargetKey));
            if (Player == Actor)
                Blackboard->ClearValue(TargetKey);
        }
    }
}

// AI 정지
void ARLEnemyAIController::ShutdownAI()
{
    // 비헤이비어 트리 정지
    if (UBrainComponent* Brain = GetBrainComponent())
    {
        Brain->StopLogic(TEXT("PlayerDead"));
    }

    // 추적・이동 정지
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

// 새로운 BT로 교체하는 함수
void ARLEnemyAIController::SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree)
{
    if (!NewBehaviorTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("SwitchBehaviorTree: NewBehaviorTree is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("SwitchBehaviorTree: Switching to new behavior tree"));

    // 1. 현재 실행 중인 BT 정지
    StopCurrentBehaviorTree();

    // 2. 잠깐 대기 (프레임 간격을 둠)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, NewBehaviorTree]()
    {
        // 3. 새로운 BT 실행
        RunBehaviorTree(NewBehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("SwitchBehaviorTree: New behavior tree started"));
    });
}

// 현재 BT 정지
void ARLEnemyAIController::StopCurrentBehaviorTree()
{
    UE_LOG(LogTemp, Log, TEXT("StopCurrentBehaviorTree: Stopping current behavior tree"));

    // 방법 1: BrainComponent를 통한 정지
    if (UBrainComponent* Brain = GetBrainComponent())
    {
        Brain->StopLogic(TEXT("BT Switch"));
    }

    // 방법 2: BehaviorTreeComponent 직접 정지
    if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(GetBrainComponent()))
    {
        BTComp->StopTree(EBTStopMode::Safe);
    }

    // 추가 정리
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

// 현재 BT 재시작
void ARLEnemyAIController::RestartCurrentBehaviorTree()
{
    if (!BehaviorTreeAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("RestartCurrentBehaviorTree: BehaviorTreeAsset is null"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("RestartCurrentBehaviorTree: Restarting current behavior tree"));

    // 현재 BT 정지
    StopCurrentBehaviorTree();

    // 다음 프레임에 재시작
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        RunBehaviorTree(BehaviorTreeAsset);
        UE_LOG(LogTemp, Log, TEXT("RestartCurrentBehaviorTree: Behavior tree restarted"));
    });
}





