// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RGCharacterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RGCharacterPlayer.h"
#include "GameInstance/RGGameInstance.h"
#include "Components/WidgetComponent.h"
#include "UI/HPWidget.h"

ARGCharacterEnemy::ARGCharacterEnemy()
{
    // 메시가 컨트롤러 회전에 맞춰 돌아보도록
    bUseControllerRotationYaw = true;

    Money = 50;

    // 적 체력바 UI
    static ConstructorHelpers::FClassFinder<UUserWidget> WBPClass(
        TEXT("/Game/Assassin/UI/WBP_HpBar"));
    if (WBPClass.Succeeded())
    {
        // 컴포넌트 생성 및 위치 배치
        HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
        HealthBarComponent->SetupAttachment(GetMesh());
        HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
        HealthBarComponent->SetDrawSize(FVector2D(200, 20));
        HealthBarComponent->SetRelativeLocation(FVector(0, 0, 200));

        // 컴포넌트가 생성할 위젯 지정
        HealthBarComponent->SetWidgetClass(WBPClass.Class);
        UE_LOG(LogTemp, Log, TEXT("WBPClass Load Success"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("WBPClass Load Failed"));
    }
}

FGenericTeamId ARGCharacterEnemy::GetGenericTeamId() const
{
    return FGenericTeamId(TeamID);
}

void ARGCharacterEnemy::TakeCharacterDamage(int32 RecieveDamage)
{
    Super::TakeCharacterDamage(RecieveDamage);
    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARGCharacterEnemy::TakeCharacterHeal(int32 RecieveHealAmount)
{
    Super::TakeCharacterHeal(RecieveHealAmount);
    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARGCharacterEnemy::BeginPlay()
{
    Super::BeginPlay();

    // 맵에서 플레이어 찾기
    Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

    if (UUserWidget* UserWidget = HealthBarComponent->GetUserWidgetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("make UUserWidget"));
        if (UHPWidget* HealthBarWidget = Cast<UHPWidget>(UserWidget))
        {
            // 델리게이트로 체력 변화 통보
            EnemyHpChange.AddUObject(HealthBarWidget, &UHPWidget::CalculateHp);
            CharacterDie.AddUObject(HealthBarWidget, &UHPWidget::DestroyWidget);

            // ... 위젯 바인딩 코드 ...
            GetWorldTimerManager().SetTimerForNextTick([this]()
                {
                    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
                });

            UE_LOG(LogTemp, Warning, TEXT("make UHPWidget"));
        }
    }
}

void ARGCharacterEnemy::Die()
{
    Super::Die();

    Player->SetMoney(Player->GetMoney() + GetMoney());

    if (Controller)
    {
        Controller->Destroy();
    }
    Player->PrintMoney();
    GetWorldTimerManager().SetTimer(DieTimerHandle, this, &ARGCharacterEnemy::DestoryCharacter, 10.0f, false);

    Player->SubtractionWorldAliveEnemyCount();

    UE_LOG(LogTemp, Warning, TEXT("LevelName WorldAliveEnemys Set : %d"), Player->GetWorldAliveEnemyCount());

    if (Player->GetWorldAliveEnemyCount() == 0)
    {
        Player->ShowStageClearWidget();
    }
}

void ARGCharacterEnemy::DestoryCharacter()
{
    Destroy();
}
