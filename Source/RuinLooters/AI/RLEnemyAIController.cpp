// EnemyAIController.cpp
#include "AI/RLEnemyAIController.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

// 블랙보드 키 상수들 정의
const FName ARLEnemyAIController::TargetKey(TEXT("TargetKey"));
const FName ARLEnemyAIController::bInRangeKey(TEXT("bInRangeKey"));
const FName ARLEnemyAIController::StartSkyPointKey(TEXT("StartSkyPointKey"));
const FName ARLEnemyAIController::CurrentSkyPointKey(TEXT("CurrentSkyPointKey"));

ARLEnemyAIController::ARLEnemyAIController()
{
    Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
    SightCfg = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
    SightCfg->SightRadius = 1500.f;
    SightCfg->LoseSightRadius = 1800.f;
    SightCfg->PeripheralVisionAngleDegrees = 180.f;
    Perception->ConfigureSense(*SightCfg);
    Perception->SetDominantSense(SightCfg->GetSenseImplementation());

    SightCfg->DetectionByAffiliation.bDetectEnemies = true;   // 다른 팀
    SightCfg->DetectionByAffiliation.bDetectFriendlies = false;  // 같은 팀 감지
    SightCfg->DetectionByAffiliation.bDetectNeutrals = false;  // 중립 감지

    Perception->OnTargetPerceptionUpdated.AddDynamic(
        this, &ARLEnemyAIController::OnPerceptionUpdated);
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





