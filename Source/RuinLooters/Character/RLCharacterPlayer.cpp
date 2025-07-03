// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLCharacterPlayer.h"
#include "Character/RLCharacterEnemy.h"
#include "Character/RLNPC.h"
#include "AI/RLEnemyAIController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "GameInstance/RLGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UI/NPCStoreWidget.h"
#include "UI/RLPlayerUI.h"
#include "UI/RLPlayerDeadWidget.h"
#include "UI/RLSettingsMenuWidget.h"
#include "UI/RLStageClearMessageWidget.h"
#include "UI/RLStageClearPortalWidget.h"
#include "SaveGame/RLSaveGame.h"
#include "Character/RLPlayerDataAsset.h"
#include "GameFramework/PlayerStart.h"
#include "Level/RLLevelTransferPortal.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ARLCharacterPlayer::ARLCharacterPlayer()
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

    // ���� �̸� ����
    LevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

    bStageExit = 1;
}

void ARLCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = Cast<APlayerController>(GetController());

    RowWeapon = GameInstance->GetWeaponInformation(CharacterWeaponName);

    if (RowWeapon)
    {
        // ���� ���� ����
        ChangeWeapon(RowWeapon);

        WeaponMeshComponent->SetSkeletalMesh(RowWeapon->SkeletalMesh);
        UE_LOG(LogTemp, Warning, TEXT("RowWeapon :%s"), *CharacterWeaponName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player RowWeapon Null"));
    }

    // LoadObject<>()�� �����ͳ� ��Ű�� ���鿡 ���?�ִ� .uasset �� ��Ÿ�ӿ� ã�� �޸𸮿� �ε���
    // �̱���ó�� ���� ��ζ�� ������ ȣ���ص� �ѹ��� �ε���
    // ��������Ʈ/C++�� �̸� ������ �� �� ���� ���� �����?������ �ҷ��� �� �ʼ�
    // �ʵ带 �����ϸ� ���� ������ ���� �ٲ� �� ����
    LoadAsset = LoadObject<URLPlayerDataAsset>(nullptr, TEXT("/Script/Roguelike123.RLPlayerDataAsset'/Game/Assassin/Blueprint/DA_PlayerStat.DA_PlayerStat'"));

    // DuplicateObject<>()�� ������ó�� ���� Ÿ�� �ʵ嵵 ���?�����Ǿ�, ���� ������ �ν��Ͻ��� ����
    // ��Ÿ�ӿ� ���� ������ �Ѽ����� �����鼭 ���� �� ����ϰ�?���� �� ���?    // ���� �����?�߻��ϹǷ� ���� ����
    // �ʱ� ������ ������ ���ϴ� ���� ���� ���� ��������
    PlayerStat = DuplicateObject<URLPlayerDataAsset>(LoadAsset, this);

    GetSaveGame();

    if (PlayerUIClass != nullptr)
    {
        PlayerUI = CreateWidget<URLPlayerUI>(World, PlayerUIClass);

        if (PlayerUI != nullptr)
        {
            PlayerUI->AddToViewport();
        }
    }


    SkillCoolChange.AddUObject(PlayerUI, &URLPlayerUI::SkillCoolTime);
    PlayerHpChange.AddUObject(PlayerUI, &URLPlayerUI::PlayerCalculateHp);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);

    // �� �� Ȯ�� �� ����
    if (!LevelName.IsNone())
    {
        // �������� �� ���� ���ʹ� ���� ���� �� üũ
        if (LevelName.ToString().Contains(TEXT("Stage")))
        {
            if (CheckEnemy())
            {
                // �� o
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

void ARLCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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

        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ARLCharacterBase::Attack);
        EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::ApplySpeedBuff);
        EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::Interaction);
        // ESC
        EnhancedInputComponent->BindAction(SettingsAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::ViewSettingWidget);

    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
    }
}

void ARLCharacterPlayer::Die()
{
    ARLCharacterBase::Die();

    // �� AI Controller ��ü �ٿ�
    for (TActorIterator<ARLEnemyAIController> It(GetWorld()); It; ++It)
    {
        ARLEnemyAIController* AIController = Cast<ARLEnemyAIController>(*It);
        if (!AIController) continue;

        AIController->ShutdownAI();
    }

    if (PlayerDieUIClass != nullptr)
    {
        PlayerDieUI = CreateWidget<URLPlayerDeadWidget>(GetWorld(), PlayerDieUIClass);

        if (PlayerDieUI != nullptr)
        {
            PlayerDieUI->AddToViewport();

            PlayerController->bShowMouseCursor = true;

            FInputModeUIOnly UIOnly;
            UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
            PlayerController->SetInputMode(UIOnly);
        }
    }

    // ȭ�� ��Ӱ�?    // ���̵� ��: 0 �� 1 alpha, ȸ��
    if (PlayerController)// = UGameplayStatics::GetPlayerController(this, 0)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }

        // CameraManager ���� ���̵� ȿ��
        PlayerController->PlayerCameraManager->StartCameraFade(
            0.0f,                // From Alpha
            0.6f,                // To Alpha
            2.0f,               // Duration
            FLinearColor::Black, // FadeColor
            false,              // bShouldFadeAudio
            true                // bHoldWhenFinished(������ ����)
        );
    }

    bStageExit = 0;
}

void ARLCharacterPlayer::Interaction()
{
    // NPC���� ��ȣ�ۿ�
    if (bIsCharacterInteractWithNPC)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }

        // ���?NPC UI ����
        InteractiveNPC->GetActiveStoreWidget()->AddToViewport(0);

        InteractiveNPC->GetActiveStoreWidget()->SetVisibility(ESlateVisibility::Visible);


        PlayerController->bShowMouseCursor = true;

        FInputModeUIOnly UIOnly = FInputModeUIOnly();
        UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
        PlayerController->SetInputMode(UIOnly);

    }
}

void ARLCharacterPlayer::ApplySpeedBuff()
{
    if (IsCanSkill == true)
    {
        IsCanSkill = false;

        AttackSpeed = 1.5;

        // 1. ���� �ӵ� ����
        OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

        // 2. ������ �ӵ��� ����
        GetCharacterMovement()->MaxWalkSpeed = 800;

        // 3. ���� Ÿ�̸Ӱ� ���� ������ Ŭ����
        GetWorldTimerManager().ClearTimer(SpeedBuffTimerHandle);

        // 4. Duration �� �� RestoreOriginalSpeed() ȣ�� ����
        GetWorldTimerManager().SetTimer(
            SpeedBuffTimerHandle,
            this,
            &ARLCharacterPlayer::RestoreOriginalSpeed,
            5.0f,
            false  // ݺ 
        );

        GetWorldTimerManager().SetTimer(
            CoolTimerHandle,
            this,
            &ARLCharacterPlayer::OnSkill,
            8.0f,
            false  // ݺ 
        );
        UE_LOG(LogTemp, Warning, TEXT("Skill On!"));

        SkillCoolChange.Broadcast(IsCanSkill);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Skill is CoolTime!"));
    }
}

void ARLCharacterPlayer::RestoreOriginalSpeed()
{
    // ���� �ӵ� ����
    GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
    AttackSpeed = 1.0;
}

void ARLCharacterPlayer::OnSkill()
{
    UE_LOG(LogTemp, Warning, TEXT("You Can Use Skill!"));
    IsCanSkill = true;

    SkillCoolChange.Broadcast(IsCanSkill);
}

FGenericTeamId ARLCharacterPlayer::GetGenericTeamId() const
{
    return FGenericTeamId(TeamID);
}

void ARLCharacterPlayer::TakeCharacterDamage(int32 RecieveDamage)
{
    ARLCharacterBase::TakeCharacterDamage(RecieveDamage);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARLCharacterPlayer::TakeCharacterHeal(int32 RecieveHealAmount)
{
    ARLCharacterBase::TakeCharacterHeal(RecieveHealAmount);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARLCharacterPlayer::TakeCharacterMaxHealth(int32 UpScale)
{
    int32 TotalMaxHp = GetMaxHp() + UpScale;
    SetCurrentHp(GetCurrentHp() + UpScale);
    SetMaxHp(TotalMaxHp);
}

void ARLCharacterPlayer::TakeCharacterDefence(int32 UpScale)
{
    int32 TotalDefence = GetDefence() + UpScale;
    SetDefence(TotalDefence);
}

void ARLCharacterPlayer::ViewSettingWidget()
{
    if (SettingsWidgetClass != nullptr)
    {
        SettingsWidget = CreateWidget<URLSettingsMenuWidget>(GetWorld(), SettingsWidgetClass);

        if (SettingsWidget != nullptr)
        {
            SettingsWidget->CheckLevelName(LevelName);

            SettingsWidget->AddToViewport();
            //  Ͻ
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

// Player UI�� �ִ� Money �� ����
void ARLCharacterPlayer::PrintMoney()
{
    if (PlayerUI != nullptr)
    {
        // FText : String�� ������ �𸮾� ���� 
        PlayerUI->MoneyData->SetText(FText::AsNumber(Money));
    }
}

// Save Data ������
void ARLCharacterPlayer::GetSaveGame()
{
    // ���̺� ���� ã��
    URLSaveGame* LoadData = GameInstance->LoadSaveGameData();

    if (LoadData)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetSaveGame() Money : %d"), LoadData->PlayerMoney);
        //  
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

//  ̵  ÷̾   
void ARLCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Ÿ̸ Ŭ
    GetWorldTimerManager().ClearAllTimersForObject(this);

    // ü ʱȭ ƴ    ư ʹٸ CurrentHp > 0 ֱ
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

void ARLCharacterPlayer::SetPlayerStat()
{
    if (CurrentHp <= 0)
    {
        // ÷̾�?׾ ʱȭ
        PlayerStat = DuplicateObject<URLPlayerDataAsset>(LoadAsset, this);
        UE_LOG(LogTemp, Warning, TEXT("PlayerStat Reset"));
    }
    else
    {
        //    ¿  ÷̾  
        PlayerStat->PlayerMoney = Money;
        PlayerStat->MaxHp = MaxHp;
        PlayerStat->CurrentHp = CurrentHp;
        PlayerStat->AttackDamage = AttackDamage;
        PlayerStat->Defence = Defence;
        PlayerStat->SaveWeaponName = CharacterWeaponName;
        PlayerStat->StageIndex = StageIndex;
    }

}

void ARLCharacterPlayer::ShowStageClearWidget()
{

	if (StageClearWidgetClass != nullptr)
	{
		StageClearMessageUI = CreateWidget<URLStageClearMessageWidget>(GetWorld(), StageClearWidgetClass);

		if (StageClearMessageUI != nullptr)
		{
            StageClearMessageUI->GameClear.AddUObject(this, &ARLCharacterPlayer::StageIndexUp);

            StageClearMessageUI->AddToViewport();
		}
	}

}

void ARLCharacterPlayer::ShowStagePortalWidget()
{
    if (StagePortalWidgetClass != nullptr)
    {
        StagePortalUI = CreateWidget<URLStageClearPortalWidget>(GetWorld(), StagePortalWidgetClass);

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

uint8 ARLCharacterPlayer::CheckEnemy()
{
    // ���� �� Enemy ��ȯ 
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(World, ARLCharacterEnemy::StaticClass(), FoundEnemies);

    // ʱȭ
    WorldAliveEnemys = 0;

    // AActor*& Ÿ ϸ�?ü   , AActor*б 
    for (AActor* FoundEnemy : FoundEnemies)
    {
        ARLCharacterEnemy* WorldEnemy = Cast<ARLCharacterEnemy>(FoundEnemy);

        // Enemy ǰ  
        if (WorldEnemy && WorldEnemy->GetCurrentHp() != 0)
        {
            WorldAliveEnemys++;
        }
    }

    // Enemy    
    if (WorldAliveEnemys <= 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}



