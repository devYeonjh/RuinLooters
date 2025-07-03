// EnemyAIController.cpp
#include "AI/RGEnemyAIController.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/RGCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

ARGEnemyAIController::ARGEnemyAIController()
{
    Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
    SightCfg = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
    SightCfg->SightRadius = 1500.f;
    SightCfg->LoseSightRadius = 1800.f;
    SightCfg->PeripheralVisionAngleDegrees = 180.f;
    Perception->ConfigureSense(*SightCfg);
    Perception->SetDominantSense(SightCfg->GetSenseImplementation());

    SightCfg->DetectionByAffiliation.bDetectEnemies = true;   // 다른 팀만
    SightCfg->DetectionByAffiliation.bDetectFriendlies = false;  // 같은 팀 무시
    SightCfg->DetectionByAffiliation.bDetectNeutrals = false;  // 중립 무시

    Perception->OnTargetPerceptionUpdated.AddDynamic(
        this, &ARGEnemyAIController::OnPerceptionUpdated);
}

void ARGEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 맵에서 플레이어 찾기
    Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    // 로컬 raw 포인터 준비
    UBlackboardComponent* BlackboardComp = nullptr;

    // 코드와 블랙보드가 연결
    if (UseBlackboard(BlackboardAsset, BlackboardComp))
    {
        // TObjectPtr 멤버인 Blackboard 에 복사
        Blackboard = BlackboardComp;

        // 내부 AAIController::Blackboard 멤버와 연동된 상태로 트리 실행
        RunBehaviorTree(BehaviorTreeAsset);
        UE_LOG(LogTemp, Warning, TEXT("Controller OnPossess"));
    }
}


void ARGEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim)
{
    UE_LOG(LogTemp, Warning, TEXT("OnPerceptionUpdated"));
    if (!Blackboard) return;

    Player = Cast<ARGCharacterPlayer>(Actor);
    if (Player)
    {
        // Stim: 자극에 대한 정보를 담은 객체
        if (Stim.WasSuccessfullySensed())
        {
            // "TargetKey"의 키에 값을 설정
            Blackboard->SetValueAsObject(TEXT("TargetKey"), Player);
            UE_LOG(LogTemp, Warning, TEXT("Enemy Detected Player"));
        }
        // 시야에서 잃었다면
        else
        {
            Player = Cast<ARGCharacterPlayer>(Blackboard->GetValueAsObject(TEXT("TargetKey")));
            if (Player == Actor)
                Blackboard->ClearValue(TEXT("TargetKey"));
        }
    }
}

// AI 정지
void ARGEnemyAIController::ShutdownAI()
{
    // 비헤이비어 트리 정지
    if (UBrainComponent* Brain = GetBrainComponent())
    {
        Brain->StopLogic(TEXT("PlayerDead"));
    }

    // 길찾기·이동 정지
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}
