// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLCharacterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"
#include "GameInstance/RLGameInstance.h"
#include "Components/WidgetComponent.h"
#include "UI/HPWidget.h"
#include "Engine/DamageEvents.h"
#include "Weapon/RLSword.h"

ARLCharacterEnemy::ARLCharacterEnemy()
{
    // 메시가 컨트롤러 회전을 따라 회전하도록
    bUseControllerRotationYaw = true;

	Money = 50;

	// HP 위젯 위치 기본값 설정
	HealthBarLocation = FVector(0, 0, 0);

	// HP 체력바 UI
	// 컴포넌트 생성 및 위치 설정
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(GetMesh());
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(200, 20));

	// 위젯 클래스는 BeginPlay에서 설정 (HpWidgetClass가 nullptr일 수 있음)
	// HealthBarComponent->SetWidgetClass(HpWidgetClass);

}

FGenericTeamId ARLCharacterEnemy::GetGenericTeamId() const
{
    return FGenericTeamId(TeamID);
}

float ARLCharacterEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    float PreviousHp = CurrentHp;
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // 체력이 감소했고 위젯이 아직 설정되지 않았다면 설정
    if (MaxHp > CurrentHp && !HpWidget)
    {
        SetupHealthBarWidget();
    }
    
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

}

void ARLCharacterEnemy::SetupHealthBarWidget()
{
    if (!HealthBarComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthBarComponent is null"));
        return;
    }

    // HpWidgetClass가 설정되어 있는지 확인
    if (!HpWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("HpWidgetClass is not set for %s"), *GetName());
        return;
    }

    HealthBarComponent->SetRelativeLocation(HealthBarLocation);

    // 위젯 클래스 설정
    HealthBarComponent->SetWidgetClass(HpWidgetClass);

    // 위젯 객체 생성 및 가져오기
    UUserWidget* UserWidget = HealthBarComponent->GetUserWidgetObject();
    if (!UserWidget)
    {
        // 위젯이 생성되지 않았다면 강제로 생성
        HealthBarComponent->InitWidget();
        UserWidget = HealthBarComponent->GetUserWidgetObject();
    }

    if (UserWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Successfully created UUserWidget for %s"), *GetName());
        
        // UHPWidget으로 캐스팅
        if (UHPWidget* HealthBarWidget = Cast<UHPWidget>(UserWidget))
        {
            // HpWidget 참조 저장
            HpWidget = HealthBarWidget;
            
            // 델리게이트에 체력 변화 바인딩
            EnemyHpChange.AddUObject(HealthBarWidget, &UHPWidget::CalculateHp);
            CharacterDie.AddUObject(HealthBarWidget, &UHPWidget::DestroyWidget);

            // 초기 체력 값 설정
            GetWorldTimerManager().SetTimerForNextTick([this]()
            {
                EnemyHpChange.Broadcast(CurrentHp, MaxHp);
            });

            UE_LOG(LogTemp, Warning, TEXT("Successfully setup UHPWidget for %s"), *GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast UserWidget to UHPWidget for %s"), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create or get UserWidget for %s"), *GetName());
    }
}

void ARLCharacterEnemy::Die()
{
    Super::Die();

    Player->SetMoney(Player->GetMoney() + GetMoney());

    // 무기 정보를 가져와서 WeaponBP 스폰
    if (GameInstance)
    {
        if (FWeaponTableRow* WeaponInfo = GameInstance->GetWeaponInformation(CharacterWeaponName))
        {
            if (WeaponInfo->WeaponBP)
            {
                FVector SpawnLocation = GetActorLocation();
                FRotator SpawnRotation = GetActorRotation();
                GetWorld()->SpawnActor<ARLSword>(WeaponInfo->WeaponBP, SpawnLocation, SpawnRotation);
            }
        }
    }

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



