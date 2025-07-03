// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLCharacterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"
#include "GameInstance/RLGameInstance.h"
#include "Components/WidgetComponent.h"
#include "UI/HPWidget.h"

ARLCharacterEnemy::ARLCharacterEnemy()
{
    // �޽ð� ��Ʈ�ѷ� ȸ���� ���� ���ƺ�����
    bUseControllerRotationYaw = true;

    Money = 50;

    // �� ü�¹� UI
    static ConstructorHelpers::FClassFinder<UUserWidget> WBPClass(
        TEXT("/Game/Assassin/UI/WBP_HpBar"));
    if (WBPClass.Succeeded())
    {
        // ������Ʈ ���� �� ��ġ ��ġ
        HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
        HealthBarComponent->SetupAttachment(GetMesh());
        HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
        HealthBarComponent->SetDrawSize(FVector2D(200, 20));
        HealthBarComponent->SetRelativeLocation(FVector(0, 0, 200));

        // ������Ʈ�� ������ ���� ����
        HealthBarComponent->SetWidgetClass(WBPClass.Class);
        UE_LOG(LogTemp, Log, TEXT("WBPClass Load Success"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WBPClass Load Failed"));
    }
}

FGenericTeamId ARLCharacterEnemy::GetGenericTeamId() const
{
    return FGenericTeamId(TeamID);
}

void ARLCharacterEnemy::TakeCharacterDamage(int32 RecieveDamage)
{
    Super::TakeCharacterDamage(RecieveDamage);
    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARLCharacterEnemy::TakeCharacterHeal(int32 RecieveHealAmount)
{
    Super::TakeCharacterHeal(RecieveHealAmount);
    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARLCharacterEnemy::BeginPlay()
{
    Super::BeginPlay();

    // �ʿ��� �÷��̾� ã��
    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

    if (UUserWidget* UserWidget = HealthBarComponent->GetUserWidgetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("make UUserWidget"));
        if (UHPWidget* HealthBarWidget = Cast<UHPWidget>(UserWidget))
        {
            // ��������Ʈ�� ü�� ��ȭ �뺸
            EnemyHpChange.AddUObject(HealthBarWidget, &UHPWidget::CalculateHp);
            CharacterDie.AddUObject(HealthBarWidget, &UHPWidget::DestroyWidget);

            // ... ���� ���ε� �ڵ� ...
            GetWorldTimerManager().SetTimerForNextTick([this]()
                {
                    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
                });

            UE_LOG(LogTemp, Warning, TEXT("make UHPWidget"));
        }
    }
}

void ARLCharacterEnemy::Die()
{
    Super::Die();

    Player->SetMoney(Player->GetMoney() + GetMoney());

    if (Controller)
    {
        Controller->Destroy();
    }
    Player->PrintMoney();
    GetWorldTimerManager().SetTimer(DieTimerHandle, this, &ARLCharacterEnemy::DestoryCharacter, 10.0f, false);

    Player->SubtractionWorldAliveEnemyCount();

    UE_LOG(LogTemp, Warning, TEXT("LevelName WorldAliveEnemys Set : %d"), Player->GetWorldAliveEnemyCount());

    if (Player->GetWorldAliveEnemyCount() == 0)
    {
        Player->ShowStageClearWidget();
    }
}

void ARLCharacterEnemy::DestoryCharacter()
{
    Destroy();
}



