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
#include "../Pool/RLProjectilePool.h"
#include "../Projectile/RLProjectile.h"
#include "../Projectile/RLPlayerProjectile.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpringArmComponent.h"

ARLCharacterPlayer::ARLCharacterPlayer()
{
    IsCanSkill = true;
    bIsCharacterInteractWithNPC = false;

    // 현재 이름 저장
    LevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

    bStageExit = 1;

    // 투사체 스킬 관련 초기화
    bCanUseProjectileSkill = true;
    bIsUsingProjectileSkill = false;
    ProjectileDamage = 40;
    ProjectileSpeed = 3000.0f;
    ProjectileSkillCooldown = 3;
    PlayerProjectilePool = nullptr;
    
    // 투사체 콜리전 설정 초기화
    ProjectileCollisionRadius = 15.0f;   // 플레이어 투사체 반지름
    ProjectileCollisionHeight = 200.0f;  // 플레이어 투사체 높이 (사용자가 200.0f로 수정함)
    GliderComponent = CreateDefaultSubobject<URLGliderComponent>(TEXT("GliderComponent"));
}

void ARLCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = Cast<APlayerController>(GetController());

    RowWeapon = GameInstance->GetWeaponInformation(CharacterWeaponName);

    if (RowWeapon)
    {
        // 무기 변경 적용
        ChangeWeapon(RowWeapon);

        WeaponMeshComponent->SetSkeletalMesh(RowWeapon->SkeletalMesh);
        UE_LOG(LogTemp, Warning, TEXT("RowWeapon :%s"), *CharacterWeaponName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player RowWeapon Null"));
    }

    // LoadObject<>()는 하드코딩된 경로에 있는 .uasset 을 런타임에 찾아 메모리에 로드함
    // 싱글톤처럼 한번 로드를 하면 호출해도 한번만 로드됨
    // 블루프린트/C++의 이름이 바뀌면 이 것 때문에 경로 찾기가 불가능하므로 불러올 수 없음
    // 필드를 포인터로 사용하면 런타임에 경로 바꿀 수 있음
    LoadAsset = LoadObject<URLPlayerDataAsset>(nullptr, TEXT("/Script/RuinLooters.RLPlayerDataAsset'/Game/Player/DA_PlayerStat.DA_PlayerStat'"));

    // DuplicateObject<>()는 원본처럼 특정 타입 필드도 모두 복사되어, 새로운 복사본 인스턴스를 생성
    // 런타임에 값을 데이터를 수정하면서 저장할 수 있고 매번 새로운 객체를 생성
    // 포인터 참조가 발생하므로 연결 끊기
    // 초기 데이터 복사본 생성하는 것이 많은 경우 메모리 부족
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

    // 현재 레벨 확인 및 설정
    if (!LevelName.IsNone())
    {
        // 스테이지 레벨 이름 포함되어 있는지 확인
        if (LevelName.ToString().Contains(TEXT("Stage")))
        {
            if (CheckEnemy())
            {
                // 적 존재
                UE_LOG(LogTemp, Warning, TEXT("LevelName  Get World Enemy : %d"), WorldAliveEnemys);
            }
        }
        else
        {
            bStageExit = false;
            UE_LOG(LogTemp, Warning, TEXT("LevelName Don't Include Stage"));
        }
    }

    // 플레이어 투사체 풀 초기화
    if (PlayerProjectileClass)
    {
        PlayerProjectilePool = NewObject<URLProjectilePool>(this);
        PlayerProjectilePool->InitializePool(GetWorld(), PlayerProjectileClass, 15);
        UE_LOG(LogTemp, Log, TEXT("Player projectile pool initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player PlayerProjectileClass is not set"));
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

        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::Attack);
        EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::ApplySpeedBuff);
        EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::Interaction);
        // ESC
        EnhancedInputComponent->BindAction(SettingsAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::ViewSettingWidget);
        // 투사체 스킬
        EnhancedInputComponent->BindAction(ProjectileSkillAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::UseProjectileSkill);

        // 롤(구르기) 입력 바인딩 (Shift키)
        EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::StartRoll);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::HandleJumpOrGlide);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
    }
}

void ARLCharacterPlayer::Die()
{
    ARLCharacterBase::Die();

    // 모든 AI Controller 객체 끄기
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

    // 화면 검게 하기
    // 페이드 설정: 0 은 1 alpha, 회색
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
            true                // bHoldWhenFinished(지속적 유지)
        );
    }

    bStageExit = 0;
}

void ARLCharacterPlayer::Interaction()
{
    // NPC와의 상호작용
    if (bIsCharacterInteractWithNPC)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }

        // 해당 NPC UI 생성
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
    // 구르기 중에는 스킬 입력 무시
    if (bIsRolling) return;

    if (IsCanSkill == true)
    {
        IsCanSkill = false;

        AttackSpeed = 1.5;

        // 1. 현재 속도 저장
        OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

        // 2. 스피드 버프속도로 변경
        GetCharacterMovement()->MaxWalkSpeed = 800;

        // 3. 기존 타이머가 있다면 클리어
        GetWorldTimerManager().ClearTimer(SpeedBuffTimerHandle);

        // 4. Duration 후 RestoreOriginalSpeed() 호출하기
        GetWorldTimerManager().SetTimer(
            SpeedBuffTimerHandle,
            this,
            &ARLCharacterPlayer::RestoreOriginalSpeed,
            5.0f,
            false  // 반복 
        );

        GetWorldTimerManager().SetTimer(
            CoolTimerHandle,
            this,
            &ARLCharacterPlayer::OnSkill,
            8.0f,
            false  // 반복 
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
    // 원래 속도 복원
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
    if (bIsInvincible)
    {
        // 무적 중이면 데미지 무시
        return;
    }
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
            // 일시정지
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

// Player UI에 있는 Money 값 출력
void ARLCharacterPlayer::PrintMoney()
{
    if (PlayerUI != nullptr)
    {
        // FText : String을 언리얼 엔진 텍스트로 변환
        PlayerUI->MoneyData->SetText(FText::AsNumber(Money));
    }
}

// Save Data 불러오기
void ARLCharacterPlayer::GetSaveGame()
{
    // 세이브 게임 찾기
    URLSaveGame* LoadData = GameInstance->LoadSaveGameData();

    if (LoadData)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetSaveGame() Money : %d"), LoadData->PlayerMoney);
        // 로드 데이터 적용
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

// 레벨 이동시 플레이어 데이터 저장
void ARLCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 타이머 클리어
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
        // 플레이어가 죽었을때 초기화
        PlayerStat = DuplicateObject<URLPlayerDataAsset>(LoadAsset, this);
        UE_LOG(LogTemp, Warning, TEXT("PlayerStat Reset"));
    }
    else
    {
        // 레벨 이동등 살아있을 때 플레이어 데이터 저장
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
    // 현재 살아있는 Enemy 반환
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(World, ARLCharacterEnemy::StaticClass(), FoundEnemies);

    // 초기화
    WorldAliveEnemys = 0;

    // AActor*& 타입이므로 참조형 구조체이지만, AActor*로 받기
    for (AActor* FoundEnemy : FoundEnemies)
    {
        ARLCharacterEnemy* WorldEnemy = Cast<ARLCharacterEnemy>(FoundEnemy);

        // Enemy 체력 검사
        if (WorldEnemy && WorldEnemy->GetCurrentHp() != 0)
        {
            WorldAliveEnemys++;
        }
    }

    // Enemy 존재 여부 반환
    if (WorldAliveEnemys <= 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void ARLCharacterPlayer::UseProjectileSkill()
{
    // 투사체 스킬 사용 가능 여부 확인
    if (!bCanUseProjectileSkill || GetCharacterMovement()->IsFalling())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player projectile skill is on cooldown"));
        return;
    }

    // 살아있는 상태 확인
    if (CurrentHp <= 0)
    {
        return;
    }

    // 투사체 스킬 사용
    bCanUseProjectileSkill = false;

    // 투사체 스킬 사용 중 상태로 변경
    bIsUsingProjectileSkill = true;

    // 애니메이션 몽타주 재생
    if (ProjectileSkillMontage && AnimInstance)
    {
        // 몽타주 종료 콜백 설정
        MontageEndedDelegate.BindUObject(this, &ARLCharacterPlayer::OnProjectileSkillMontageEnded);
        
        // 몽타주 재생 및 델리게이트 설정
        float MontageLength = AnimInstance->Montage_Play(ProjectileSkillMontage);
        AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ProjectileSkillMontage);
        
        GetCharacterMovement()->SetMovementMode(MOVE_None);
        
        UE_LOG(LogTemp, Warning, TEXT("Player projectile skill montage started - Length: %f"), MontageLength);
    }

    // 쿨다운 시작 (람다함수 사용)
    GetWorldTimerManager().SetTimer(
        ProjectileSkillCooldownHandle,[this]()
        {
            bCanUseProjectileSkill = true;
            UE_LOG(LogTemp, Log, TEXT("Player projectile skill cooldown finished"));
        }, ProjectileSkillCooldown, false
    );
}

void ARLCharacterPlayer::FirePlayerProjectile()
{
    if (!PlayerProjectilePool)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player projectile pool is not initialized"));
        return;
    }

    // 투사체 풀에서 투사체 가져오기 (ARLPlayerProjectile로 캐스트)
    ARLPlayerProjectile* PlayerProjectile = Cast<ARLPlayerProjectile>(PlayerProjectilePool->GetProjectile());
    if (!PlayerProjectile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get player projectile from pool"));
        return;
    }

    // 발사 위치 및 방향 설정
    FVector PlayerLocation = GetActorLocation();
    FVector FireLocation = PlayerLocation + FVector(0.0f, 0.0f, 220.0f);
    
    // 카메라가 바라보는 방향으로 발사 (컨트롤러 회전 기준)
    FVector FireDirection = PlayerController ? PlayerController->GetControlRotation().Vector() : GetActorForwardVector();

    // 투사체 가시성 및 콜리전 활성화 (풀에서 가져온 경우 숨겨져 있을 수 있음)
    PlayerProjectile->SetActorHiddenInGame(false);
    PlayerProjectile->SetActorEnableCollision(true);

    // 플레이어 설정값으로 투사체 설정 (캐릭터에서 설정한 콜리전 값 사용)
    PlayerProjectile->SetupWithPlayerSettings(
        ProjectileCollisionRadius,  // 캐릭터에서 설정한 반지름
        ProjectileCollisionHeight,  // 캐릭터에서 설정한 높이
        ProjectileDamage,           // 캐릭터에서 설정한 데미지
        ProjectileSpeed             // 캐릭터에서 설정한 속도
    );

    // 투사체 초기화 및 발사
    PlayerProjectile->InitializeProjectile(FireLocation, FireDirection, PlayerProjectile->GetProjectileSettings());

    // 투사체 소유자 설정
    PlayerProjectile->SetOwner(this);

    UE_LOG(LogTemp, Warning, TEXT("Player projectile fired - Location: %s, Direction: %s, Radius: %f, Height: %f"), 
           *FireLocation.ToString(), *FireDirection.ToString(), ProjectileCollisionRadius, ProjectileCollisionHeight);

    // 투사체 반환 처리를 위한 타이머 (생존 시간 후 자동 반환)
    FTimerHandle ReturnTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(ReturnTimerHandle, [this, PlayerProjectile]()
    {
        if (PlayerProjectilePool && PlayerProjectile)
        {
            PlayerProjectilePool->ReturnProjectile(PlayerProjectile);
        }
    }, 30.0f, false);  // 플레이어 투사체의 생존 시간에 맞춤

    UE_LOG(LogTemp, Log, TEXT("Player projectile fired with custom collision settings"));
}

void ARLCharacterPlayer::OnProjectileSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("OnProjectileSkillMontageEnded called! Interrupted: %s"), bInterrupted ? TEXT("true") : TEXT("false"));

    // 몽타주 종료 시 이동 모드 복원
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    bIsUsingProjectileSkill = false;

    UE_LOG(LogTemp, Warning, TEXT("Player projectile skill montage ended - Movement restored"));
}

void ARLCharacterPlayer::HandleJumpOrGlide()
{
    if (GliderComponent && GliderComponent->GetOwner() == this && !GliderComponent->IsGliderActive() && GetCharacterMovement()->IsFalling())
    {
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 1000.0f);
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
        float Height = bHit ? (Start.Z - Hit.Location.Z) : 1000.0f;
        if (Height > 500.0f) // 200cm 이상일 때만 글라이더 진입 허용
        {
            GliderComponent->ActivateGlider();
            return;
        }
    }
    Super::Jump();
}

void ARLCharacterPlayer::StartRoll()
{
    if (GliderComponent && GliderComponent->IsGliderActive())
    {
        return;
    }
    Super::StartRoll();
}

void ARLCharacterPlayer::Attack()
{
    if (GliderComponent && GliderComponent->IsGliderActive())
    {
        return;
    }

    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

    Super::Attack();
}

void ARLCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (GliderComponent && GliderComponent->IsGliderActive())
    {
        FVector Start = GetActorLocation();
        FVector End = Start - FVector(0, 0, 120.0f); // 120cm downward
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
        if (bHit)
        {
            GliderComponent->DeactivateGlider();
        }
        // Print velocity for debugging
        FVector Velocity = GetCharacterMovement()->Velocity;
    }
    if (CameraBoom)
    {
        float InterpSpeed = 3.0f; // 부드러운 속도
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, CameraTargetArmLength, DeltaTime, InterpSpeed);
    }
}



