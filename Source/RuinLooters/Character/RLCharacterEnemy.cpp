// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLCharacterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"
#include "GameInstance/RLGameInstance.h"
#include "Components/WidgetComponent.h"
#include "UI/HPWidget.h"
#include "Engine/DamageEvents.h"

ARLCharacterEnemy::ARLCharacterEnemy()
{
    // 메시가 컨트롤러 회전을 따라 회전하도록
    bUseControllerRotationYaw = true;

	Money = 50;

	// HP 체력바 UI
	// 컴포넌트 생성 및 위치 설정
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(GetMesh());
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(200, 20));
	HealthBarComponent->SetRelativeLocation(FVector(0, 0, 200));

	// 컴포넌트에 위젯을 설정 지정
	HealthBarComponent->SetWidgetClass(HpWidgetClass);

}

FGenericTeamId ARLCharacterEnemy::GetGenericTeamId() const
{
    return FGenericTeamId(TeamID);
}

float ARLCharacterEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
    return ActualDamage;
}

void ARLCharacterEnemy::TakeCharacterHeal(int32 RecieveHealAmount)
{
    Super::TakeCharacterHeal(RecieveHealAmount);
    EnemyHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARLCharacterEnemy::BeginPlay()
{
    Super::BeginPlay();

    // 필요한 플레이어 찾기
    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

    if (UUserWidget* UserWidget = HealthBarComponent->GetUserWidgetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("make UUserWidget"));
        if (UHPWidget* HealthBarWidget = Cast<UHPWidget>(UserWidget))
        {
            // 델리게이트에 체력 변화 바인딩
            EnemyHpChange.AddUObject(HealthBarWidget, &UHPWidget::CalculateHp);
            CharacterDie.AddUObject(HealthBarWidget, &UHPWidget::DestroyWidget);

            // ... 기타 바인딩 코드 ...
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

void ARLCharacterEnemy::CallAttackCollision()
{
    ARLCharacterBase::CallAttackCollision();
}



