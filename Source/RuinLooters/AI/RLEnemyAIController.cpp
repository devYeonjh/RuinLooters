// EnemyAIController.cpp
#include "AI/RLEnemyAIController.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

ARLEnemyAIController::ARLEnemyAIController()
{
    Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
    SightCfg = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
    SightCfg->SightRadius = 1500.f;
    SightCfg->LoseSightRadius = 1800.f;
    SightCfg->PeripheralVisionAngleDegrees = 180.f;
    Perception->ConfigureSense(*SightCfg);
    Perception->SetDominantSense(SightCfg->GetSenseImplementation());

    SightCfg->DetectionByAffiliation.bDetectEnemies = true;   // �ٸ� ����
    SightCfg->DetectionByAffiliation.bDetectFriendlies = false;  // ���� �� ����
    SightCfg->DetectionByAffiliation.bDetectNeutrals = false;  // �߸� ����

    Perception->OnTargetPerceptionUpdated.AddDynamic(
        this, &ARLEnemyAIController::OnPerceptionUpdated);
}

void ARLEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // �ʿ��� �÷��̾� ã��
    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    // ���� raw ������ �غ�
    UBlackboardComponent* BlackboardComp = nullptr;

    // �ڵ��?�������尡 ����
    if (UseBlackboard(BlackboardAsset, BlackboardComp))
    {
        // TObjectPtr �����?Blackboard �� ����
        Blackboard = BlackboardComp;

        // ���� AAIController::Blackboard �����?������ ���·� Ʈ�� ����
        RunBehaviorTree(BehaviorTreeAsset);
        UE_LOG(LogTemp, Warning, TEXT("Controller OnPossess"));
    }
}


void ARLEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stim)
{
    UE_LOG(LogTemp, Warning, TEXT("OnPerceptionUpdated"));
    if (!Blackboard) return;

    Player = Cast<ARLCharacterPlayer>(Actor);
    if (Player)
    {
        // Stim: �ڱؿ� ���� ������ ���� ��ü
        if (Stim.WasSuccessfullySensed())
        {
            // "TargetKey"�� Ű�� ���� ����
            Blackboard->SetValueAsObject(TEXT("TargetKey"), Player);
            UE_LOG(LogTemp, Warning, TEXT("Enemy Detected Player"));
        }
        // �þ߿��� �Ҿ��ٸ�
        else
        {
            Player = Cast<ARLCharacterPlayer>(Blackboard->GetValueAsObject(TEXT("TargetKey")));
            if (Player == Actor)
                Blackboard->ClearValue(TEXT("TargetKey"));
        }
    }
}

// AI ����
void ARLEnemyAIController::ShutdownAI()
{
    // �����̺��?Ʈ�� ����
    if (UBrainComponent* Brain = GetBrainComponent())
    {
        Brain->StopLogic(TEXT("PlayerDead"));
    }

    // ��ã�⡤�̵� ����
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}



