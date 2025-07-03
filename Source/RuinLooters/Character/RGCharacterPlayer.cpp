// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RGCharacterPlayer.h"
#include "Character/RGCharacterEnemy.h"
#include "Character/RGNPC.h"
#include "AI/RGEnemyAIController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "GameInstance/RGGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UI/NPCStoreWidget.h"
#include "UI/RGPlayerUI.h"
#include "UI/RGPlayerDeadWidget.h"
#include "UI/RGSettingsMenuWidget.h"
#include "UI/RGStageClearMessageWidget.h"
#include "UI/RGStageClearPortalWidget.h"
#include "SaveGame/RGSaveGame.h"
#include "Character/RGPlayerDataAsset.h"
#include "GameFramework/PlayerStart.h"
#include "Level/RGLevelTransferPortal.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ARGCharacterPlayer::ARGCharacterPlayer()
{
    IsCanSkill = true;
    bIsCharacterInteractWithNPC = false;

    // Player UI Ref
    static ConstructorHelpers::FClassFinder<UUserWidget> PlayerUIClassRef(TEXT("/Game/Assassin/UI/WBP_PlayerUIWidget.WBP_PlayerUIWidget_C"));
    if (PlayerUIClassRef.Succeeded())
    {
        PlayerUIClass = PlayerUIClassRef.Class;
    }

    // SettingWidget Ref
    static ConstructorHelpers::FClassFinder<UUserWidget> SettingsWidgetClassRef(TEXT("/Game/Assassin/UI/WBP_SettingWidget.WBP_SettingWidget_C"));
    if (SettingsWidgetClassRef.Succeeded())
    {
        SettingsWidgetClass = SettingsWidgetClassRef.Class;
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> PlayerDieUIClassRef(TEXT("/Game/Assassin/UI/WBP_PlayerDead.WBP_PlayerDead_C"));
    if (PlayerDieUIClassRef.Succeeded())
    {
        PlayerDieUIClass = PlayerDieUIClassRef.Class;
    }

    // StageClearWidget Ref
    static ConstructorHelpers::FClassFinder<UUserWidget> StageClearWidgetRef(TEXT("/Game/Assassin/UI/WBP_StageClearMessageWidget.WBP_StageClearMessageWidget_C"));
    if (StageClearWidgetRef.Succeeded())
    {
        StageClearWidgetClass = StageClearWidgetRef.Class;
    }

    // PortalWidget Ref
    static ConstructorHelpers::FClassFinder<UUserWidget> StagePortalWidgetClassRef(TEXT("/Game/Assassin/UI/WBP_StageClearPortalWidget.WBP_StageClearPortalWidget_C"));
    if (StagePortalWidgetClassRef.Succeeded())
    {
        StagePortalWidgetClass = StagePortalWidgetClassRef.Class;
    }

    // 레벨 이름 저장
    LevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

    bStageExit = 1;
}

void ARGCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = Cast<APlayerController>(GetController());

    RowWeapon = GameInstance->GetWeaponInformation(CharacterWeaponName);

    if (RowWeapon)
    {
        // 시작 무기 장착
        ChangeWeapon(RowWeapon);

        WeaponMeshComponent->SetSkeletalMesh(RowWeapon->SkeletalMesh);
        UE_LOG(LogTemp, Warning, TEXT("RowWeapon :%s"), *CharacterWeaponName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player RowWeapon Null"));
    }

    // LoadObject<>()는 에디터나 패키지 번들에 들어 있는 .uasset 을 런타임에 찾아 메모리에 로드함
    // 싱글톤처럼 동일 경로라면 여러번 호출해도 한번만 로드함
    // 블루프린트/C++에 미리 참조를 걸 수 없는 동적 경로의 에셋을 불러올 때 필수
    // 필드를 수정하면 원본 에셋이 직접 바뀔 수 있음
    LoadAsset = LoadObject<URGPlayerDataAsset>(nullptr, TEXT("/Script/Roguelike123.RGPlayerDataAsset'/Game/Assassin/Blueprint/DA_PlayerStat.DA_PlayerStat'"));

    // DuplicateObject<>()는 포인터처럼 참조 타입 필드도 재귀 복제되어, 서로 독립된 인스턴스가 생성
    // 런타임에 원본 에셋을 훼손하지 않으면서 수정 및 사용하고 싶을 때 사용
    // 복제 비용이 발생하므로 남용 주의
    // 초기 스탯이 영구히 변하는 것을 막기 위해 깊은복사
    PlayerStat = DuplicateObject<URGPlayerDataAsset>(LoadAsset, this);

    GetSaveGame();

    if (PlayerUIClass != nullptr)
    {
        PlayerUI = CreateWidget<URGPlayerUI>(World, PlayerUIClass);

        if (PlayerUI != nullptr)
        {
            PlayerUI->AddToViewport();
        }
    }


    SkillCoolChange.AddUObject(PlayerUI, &URGPlayerUI::SkillCoolTime);
    PlayerHpChange.AddUObject(PlayerUI, &URGPlayerUI::PlayerCalculateHp);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);

    // 적 수 확인 및 저장
    if (!LevelName.IsNone())
    {
        // 스테이지 일 때만 에너미 개수 저장 및 체크
        if (LevelName.ToString().Contains(TEXT("Stage")))
        {
            if (CheckEnemy())
            {
                // 적 o
                UE_LOG(LogTemp, Warning, TEXT("LevelName  Get World Enemy : %d"), WorldAliveEnemys);
            }
        }
        else
        {
            bStageExit = false;
            UE_LOG(LogTemp, Warning, TEXT("LevelName Don't Include Stage"));
        }
    }
}

void ARGCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Add Input Mapping Context
    if (PlayerController)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
        // Attacking

        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ARGCharacterBase::Attack);
        EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ARGCharacterPlayer::ApplySpeedBuff);
        EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ARGCharacterPlayer::Interaction);
        // ESC
        EnhancedInputComponent->BindAction(SettingsAction, ETriggerEvent::Started, this, &ARGCharacterPlayer::ViewSettingWidget);

    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
    }
}

void ARGCharacterPlayer::Die()
{
    ARGCharacterBase::Die();

    // 적 AI Controller 전체 다운
    for (TActorIterator<ARGEnemyAIController> It(GetWorld()); It; ++It)
    {
        ARGEnemyAIController* AIController = Cast<ARGEnemyAIController>(*It);
        if (!AIController) continue;

        AIController->ShutdownAI();
    }

    if (PlayerDieUIClass != nullptr)
    {
        PlayerDieUI = CreateWidget<URGPlayerDeadWidget>(GetWorld(), PlayerDieUIClass);

        if (PlayerDieUI != nullptr)
        {
            PlayerDieUI->AddToViewport();

            PlayerController->bShowMouseCursor = true;

            FInputModeUIOnly UIOnly;
            UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
            PlayerController->SetInputMode(UIOnly);
        }
    }

    // 화면 어둡게
    // 페이드 인: 0 → 1 alpha, 회색
    if (PlayerController)// = UGameplayStatics::GetPlayerController(this, 0)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }

        // CameraManager 통해 페이드 효과
        PlayerController->PlayerCameraManager->StartCameraFade(
            0.0f,                // From Alpha
            0.6f,                // To Alpha
            2.0f,               // Duration
            FLinearColor::Black, // FadeColor
            false,              // bShouldFadeAudio
            true                // bHoldWhenFinished(끝나면 고정)
        );
    }

    bStageExit = 0;
}

void ARGCharacterPlayer::Interaction()
{
    // NPC와의 상호작용
    if (bIsCharacterInteractWithNPC)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }

        // 즉시 NPC UI 생성
        InteractiveNPC->GetActiveStoreWidget()->AddToViewport(0);

        InteractiveNPC->GetActiveStoreWidget()->SetVisibility(ESlateVisibility::Visible);


        PlayerController->bShowMouseCursor = true;

        FInputModeUIOnly UIOnly = FInputModeUIOnly();
        UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
        PlayerController->SetInputMode(UIOnly);

    }
}

void ARGCharacterPlayer::ApplySpeedBuff()
{
    if (IsCanSkill == true)
    {
        IsCanSkill = false;

        AttackSpeed = 1.5;

        // 1. 원래 속도 저장
        OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

        // 2. 버프된 속도로 셋팅
        GetCharacterMovement()->MaxWalkSpeed = 800;

        // 3. 기존 타이머가 돌고 있으면 클리어
        GetWorldTimerManager().ClearTimer(SpeedBuffTimerHandle);

        // 4. Duration 초 후 RestoreOriginalSpeed() 호출 예약
        GetWorldTimerManager().SetTimer(
            SpeedBuffTimerHandle,
            this,
            &ARGCharacterPlayer::RestoreOriginalSpeed,
            5.0f,
            false  // 반복하지 않음
        );

        GetWorldTimerManager().SetTimer(
            CoolTimerHandle,
            this,
            &ARGCharacterPlayer::OnSkill,
            8.0f,
            false  // 반복하지 않음
        );
        UE_LOG(LogTemp, Warning, TEXT("Skill On!"));

        SkillCoolChange.Broadcast(IsCanSkill);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill is CoolTime!"));
    }
}

void ARGCharacterPlayer::RestoreOriginalSpeed()
{
    // 원래 속도 복구
    GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
    AttackSpeed = 1.0;
}

void ARGCharacterPlayer::OnSkill()
{
    UE_LOG(LogTemp, Warning, TEXT("You Can Use Skill!"));
    IsCanSkill = true;

    SkillCoolChange.Broadcast(IsCanSkill);
}

FGenericTeamId ARGCharacterPlayer::GetGenericTeamId() const
{
    return FGenericTeamId(TeamID);
}

void ARGCharacterPlayer::TakeCharacterDamage(int32 RecieveDamage)
{
    ARGCharacterBase::TakeCharacterDamage(RecieveDamage);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARGCharacterPlayer::TakeCharacterHeal(int32 RecieveHealAmount)
{
    ARGCharacterBase::TakeCharacterHeal(RecieveHealAmount);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARGCharacterPlayer::TakeCharacterMaxHealth(int32 UpScale)
{
    int32 TotalMaxHp = GetMaxHp() + UpScale;
    SetCurrentHp(GetCurrentHp() + UpScale);
    SetMaxHp(TotalMaxHp);
}

void ARGCharacterPlayer::TakeCharacterDefence(int32 UpScale)
{
    int32 TotalDefence = GetDefence() + UpScale;
    SetDefence(TotalDefence);
}

void ARGCharacterPlayer::ViewSettingWidget()
{
    if (SettingsWidgetClass != nullptr)
    {
        SettingsWidget = CreateWidget<URGSettingsMenuWidget>(GetWorld(), SettingsWidgetClass);

        if (SettingsWidget != nullptr)
        {
            SettingsWidget->CheckLevelName(LevelName);

            SettingsWidget->AddToViewport();
            // 게임 일시정지
            UGameplayStatics::SetGamePaused(GetWorld(), true);

            if (PlayerController)
            {
                PlayerController->SetShowMouseCursor(true);

                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(SettingsWidget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PlayerController->SetInputMode(InputMode);
            }
        }
    }
}

// Player UI에 있는 Money 값 변경
void ARGCharacterPlayer::PrintMoney()
{
    if (PlayerUI != nullptr)
    {
        // FText : String과 유사한 언리얼 문자 
        PlayerUI->MoneyData->SetText(FText::AsNumber(Money));
    }
}

// Save Data 가져옴
void ARGCharacterPlayer::GetSaveGame()
{
    // 세이브 파일 찾기
    URGSaveGame* LoadData = GameInstance->LoadSaveGameData();

    if (LoadData)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetSaveGame() Money : %d"), LoadData->PlayerMoney);
        // 값 가져옴
        Money = LoadData->PlayerMoney;
        MaxHp = LoadData->MaxHp;
        CurrentHp = LoadData->CurrentHp;
        AttackDamage = LoadData->AttackDamage;
        Defence = LoadData->Defence;
        CharacterWeaponName = LoadData->SaveWeaponName;
        StageIndex = LoadData->StageIndex;

        RowWeapon = GameInstance->GetWeaponInformation(CharacterWeaponName);

        if (RowWeapon)
        {
            ChangeWeapon(RowWeapon);

            WeaponMeshComponent->SetSkeletalMesh(RowWeapon->SkeletalMesh);
        }
    }
}

// 레벨 이동 및 플레이어 종료 시 저장
void ARGCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 타이머 클리어
    GetWorldTimerManager().ClearAllTimersForObject(this);

    // 전체 초기화가 아닌 던전 입장 전으로 돌아가고 싶다면 CurrentHp > 0 빼주기
    if (WorldAliveEnemys >= 1 && CurrentHp > 0)
    {
        return;
    }

    if (RowWeapon)
    {
        CharacterWeaponName = GameInstance->GetWeaponName(RowWeapon->WeaponIndex);
    }

    SetPlayerStat();
    GameInstance->SetSaveGame(PlayerStat);
}

void ARGCharacterPlayer::SetPlayerStat()
{
    if (CurrentHp <= 0)
    {
        // 플레이어가 죽었으면 초기화
        PlayerStat = DuplicateObject<URGPlayerDataAsset>(LoadAsset, this);
        UE_LOG(LogTemp, Warning, TEXT("PlayerStat Reset"));
    }
    else
    {
        // 저장 시 데이터 에셋에 현재 플레이어 상태 저장
        PlayerStat->PlayerMoney = Money;
        PlayerStat->MaxHp = MaxHp;
        PlayerStat->CurrentHp = CurrentHp;
        PlayerStat->AttackDamage = AttackDamage;
        PlayerStat->Defence = Defence;
        PlayerStat->SaveWeaponName = CharacterWeaponName;
        PlayerStat->StageIndex = StageIndex;
    }

}

void ARGCharacterPlayer::ShowStageClearWidget()
{

	if (StageClearWidgetClass != nullptr)
	{
		StageClearMessageUI = CreateWidget<URGStageClearMessageWidget>(GetWorld(), StageClearWidgetClass);

		if (StageClearMessageUI != nullptr)
		{
            StageClearMessageUI->GameClear.AddUObject(this, &ARGCharacterPlayer::StageIndexUp);

            StageClearMessageUI->AddToViewport();
		}
	}

}

void ARGCharacterPlayer::ShowStagePortalWidget()
{
    if (StagePortalWidgetClass != nullptr)
    {
        StagePortalUI = CreateWidget<URGStageClearPortalWidget>(GetWorld(), StagePortalWidgetClass);

        if (StagePortalUI != nullptr)
        {
            StagePortalUI->AddToViewport();

            UGameplayStatics::SetGamePaused(GetWorld(), true);

            if (PlayerController)
            {
                PlayerController->SetShowMouseCursor(true);

                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(StagePortalUI->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PlayerController->SetInputMode(InputMode);
            }
        }
    }
}

uint8 ARGCharacterPlayer::CheckEnemy()
{
    // 월드 내 Enemy 반환 
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(World, ARGCharacterEnemy::StaticClass(), FoundEnemies);

    // 초기화
    WorldAliveEnemys = 0;

    // AActor*& 타입을 사용하면 배열 자체를 수정할 수 있음, AActor*는 읽기 전용
    for (AActor* FoundEnemy : FoundEnemies)
    {
        ARGCharacterEnemy* WorldEnemy = Cast<ARGCharacterEnemy>(FoundEnemy);

        // Enemy의 피가 있을 때
        if (WorldEnemy && WorldEnemy->GetCurrentHp() != 0)
        {
            WorldAliveEnemys++;
        }
    }

    // Enemy가 월드 내에 없을 때
    if (WorldAliveEnemys <= 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
