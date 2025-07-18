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
#include "Engine/DamageEvents.h"
#include "Camera/CameraComponent.h"
#include "Projectile/RLArrow.h"
#include "Pool/RLArrowPool.h"

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

    // 에이밍 시스템 초기화
    bIsAiming = false;
    AimingCameraDistance = 10.0f;  // 에이밍 시 카메라 거리
    NormalCameraDistance = 400.0f;  // 일반 상태 카메라 거리
    BowDrawSound = nullptr;

    // 폼 체인지 초기화 (기본적으로 검 모드)
    bIsSword = true;

    // 활 메시 컴포넌트 생성 및 초기화
    BowMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BowMesh"));
    BowMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_lBowSocket"));
    BowMeshComponent->SetSkeletalMesh(nullptr);
    BowMeshComponent->SetCastShadow(false);
    // 물리적 상호작용 완전 제거
    BowMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BowMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    BowMeshComponent->SetGenerateOverlapEvents(false);
    BowMeshComponent->SetNotifyRigidBodyCollision(false);
    BowMeshComponent->SetVisibility(false); // 기본적으로 숨김 (검 모드이므로)
    BowMeshComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f)); // 크기를 반으로 줄임

    // 화살 시스템 초기화
    bIsLoadingArrow = false;
    bIsArrowLoaded = false;
    LoadedArrow = nullptr;
    
    // 차징 시스템 초기화
    bIsChargingArrow = false;
    CurrentChargeTime = 0.0f;
    MaxChargeTime = 1.0f;  // 최대 1초 차징
    MinArrowSpeed = 300.0f;  // 최소 속도
    MaxArrowSpeed = 4000.0f; // 최대 속도
    
    // 화살 갯수 초기화
    MaxArrowCount = 30;      // 최대 화살 갯수
    CurrentArrowCount = 30;  // 현재 화살 갯수 (기본값)

    // 카메라 위치 초기화
    NormalCameraPosition = FVector(0.0f, 0.0f, 0.0f);
    AimingCameraPosition = FVector(0.0f, 50.0f, 70.0f);
}

void ARLCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = Cast<APlayerController>(GetController());
    
    // 활 메시 컴포넌트 충돌 설정 강제 적용 (블루프린트 재정의 방지)
    if (BowMeshComponent)
    {
        BowMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BowMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        BowMeshComponent->SetGenerateOverlapEvents(false);
        BowMeshComponent->SetNotifyRigidBodyCollision(false);
        UE_LOG(LogTemp, Warning, TEXT("BowMeshComponent collision forcibly disabled in BeginPlay"));
    }

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
    ArrowCountChanged.AddUObject(PlayerUI, &URLPlayerUI::UpdateArrowCount);
    
    // 에이밍 상태 변화 델리게이트 바인딩
    AimingStateChanged.AddLambda([this](bool bIsAiming)
    {
        if (PlayerUI)
        {
            if (bIsAiming)
            {
                PlayerUI->ShowArrowPoint();
            }
            else
            {
                PlayerUI->HideArrowPoint();
            }
        }
    });
    
    // 초기 화살 갯수 UI 업데이트
    ArrowCountChanged.Broadcast(CurrentArrowCount, MaxArrowCount);

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

    // 화살 클래스 확인
    if (ArrowClass)
    {
        UE_LOG(LogTemp, Log, TEXT("Arrow class is set"));
        // 화살 풀 초기화
        ArrowPool = NewObject<URLArrowPool>(this);
        ArrowPool->InitializeArrowPool(GetWorld(), ArrowClass, 10);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ArrowClass is not set"));
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
        //EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::Attack);
        // Attacking - 홀딩 지원을 위해 Started/Completed로 변경
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::OnAttackPressed);
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ARLCharacterPlayer::OnAttackReleased);
        
        EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::ApplySpeedBuff);
        EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::Interaction);
        // ESC
        EnhancedInputComponent->BindAction(SettingsAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::ViewSettingWidget);
        // 투사체 스킬
        EnhancedInputComponent->BindAction(ProjectileSkillAction, ETriggerEvent::Triggered, this, &ARLCharacterPlayer::UseProjectileSkill);
        // 롤(구르기) 입력 바인딩 (Shift키)
        EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::StartRoll);
        // 글라이더 점프
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::HandleJumpOrGlide);
        // 에이밍 시작/종료
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::StartAiming);
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ARLCharacterPlayer::StopAiming);
        // 검, 활 폼 체인지
        EnhancedInputComponent->BindAction(FormChangeAction, ETriggerEvent::Started, this, &ARLCharacterPlayer::ChangeForm);
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

        // 스피드 스킬 몽타주가 설정되어 있다면 몽타주 실행
        if (SpeedSkillMontage && AnimInstance)
        {
            // 몽타주 종료 콜백 설정
            FOnMontageEnded SpeedSkillMontageEndedDelegate;
            SpeedSkillMontageEndedDelegate.BindUObject(this, &ARLCharacterPlayer::OnSpeedSkillMontageEnded);
            
            // 몽타주 재생 및 델리게이트 설정
            float MontageLength = AnimInstance->Montage_Play(SpeedSkillMontage);
            AnimInstance->Montage_SetEndDelegate(SpeedSkillMontageEndedDelegate, SpeedSkillMontage);
            
            // 몽타주 재생 중에는 이동 제한
            GetCharacterMovement()->SetMovementMode(MOVE_None);
            
            UE_LOG(LogTemp, Warning, TEXT("Speed skill montage started - Length: %f"), MontageLength);
        }
        else
        {
            // 몽타주가 없으면 바로 스피드 버프 적용
            ApplySpeedBuffEffect();
        }

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
    MontageSpeed = 1.0;
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

float ARLCharacterPlayer::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsInvincible)
    {
        // 무적 중이면 데미지 무시
        return 0.0f;
    }
    float ActualDamage = ARLCharacterBase::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    PlayerHpChange.Broadcast(CurrentHp, MaxHp);
    return ActualDamage;
}

void ARLCharacterPlayer::TakeCharacterHeal(int32 RecieveHealAmount)
{
    ARLCharacterBase::TakeCharacterHeal(RecieveHealAmount);

    PlayerHpChange.Broadcast(CurrentHp, MaxHp);
}

void ARLCharacterPlayer::TakeCharacterMaxHealth(int32 UpScale)
{
    float TotalMaxHp = GetMaxHp() + UpScale;
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

    // 투사체 풀 정리
    if (PlayerProjectilePool)
    {
        PlayerProjectilePool->CleanupActiveProjectiles();
        UE_LOG(LogTemp, Warning, TEXT("Player EndPlay: Cleaned up projectile pool"));
    }

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
        if (PlayerProjectilePool && PlayerProjectile && IsValid(PlayerProjectile))
        {
            PlayerProjectilePool->ReturnProjectile(PlayerProjectile);
            UE_LOG(LogTemp, Log, TEXT("Player projectile auto-returned after 30 seconds"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Player projectile timer called but projectile is invalid"));
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

void ARLCharacterPlayer::OnSpeedSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("OnSpeedSkillMontageEnded called! Interrupted: %s"), bInterrupted ? TEXT("true") : TEXT("false"));

    // 몽타주 종료 시 이동 모드 복원
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    // 스피드 버프 효과 적용
    ApplySpeedBuffEffect();

    UE_LOG(LogTemp, Warning, TEXT("Speed skill montage ended - Speed buff applied"));
}

void ARLCharacterPlayer::ApplySpeedBuffEffect()
{
    MontageSpeed = 1.5;

    // 1. 현재 속도 저장
    OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

    // 2. 스피드 버프속도로 변경
    GetCharacterMovement()->MaxWalkSpeed = 1000;

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
    UE_LOG(LogTemp, Warning, TEXT("Speed Buff Effect Applied!"));
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
    if (!bIsSword || GliderComponent && GliderComponent->IsGliderActive() || GetCharacterMovement()->IsFalling())
    {
        return;
    }

    Super::Attack();
}

void ARLCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 차징 시스템 업데이트
    UpdateCharging(DeltaTime);
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
        float InterpSpeed = 4.0f; // 0.5초에 걸쳐 부드러운 전환 (2.0f 사용)
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, CameraTargetArmLength, DeltaTime, InterpSpeed);
    }

    // 활 가시성 업데이트 (폼 상태에 따라)
    if (BowMeshComponent)
    {
        BowMeshComponent->SetVisibility(!bIsSword);
    }

    // WeaponMeshComponent(검) 가시성 업데이트 (폼 상태에 따라)
    if (WeaponMeshComponent)
    {
        WeaponMeshComponent->SetVisibility(bIsSword);
    }
}

// 플레이어 입력 차단 (구르기 중)
void ARLCharacterPlayer::DisablePlayerInput()
{
    if (PlayerController)
    {
        // Enhanced Input Mapping Context 제거
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
            UE_LOG(LogTemp, Log, TEXT("Player input disabled for rolling"));
        }
    }
}

// 플레이어 입력 복원 (구르기 종료)
void ARLCharacterPlayer::EnablePlayerInput()
{
    if (PlayerController)
    {
        // Enhanced Input Mapping Context 재추가
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
            UE_LOG(LogTemp, Log, TEXT("Player input enabled after rolling"));
        }
    }
}

// 에이밍 시작
void ARLCharacterPlayer::StartAiming()
{
    // 활 모드일 때만 에이밍 가능
    if (bIsSword)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot aim in sword mode"));
        return;
    }

    // 공중에 있거나 구르기 중일 때는 에이밍 불가
    if (GetCharacterMovement()->IsFalling() || bIsRolling)
    {
        return;
    }

    bIsAiming = true;
    
    // 에이밍 상태 변화 델리게이트 브로드캐스트
    AimingStateChanged.Broadcast(true);
    
    // 카메라 위치를 에이밍 모드로 변경 (FollowCamera의 상대적 위치)
    if (FollowCamera)
    {
        FollowCamera->SetRelativeLocation(AimingCameraPosition);
    }
    
    // 에이밍 상태에서 SpringArm 길이 조정
    CameraTargetArmLength = AimingCameraDistance;
    
    // OrientRotationToMovement를 false로 설정
    GetCharacterMovement()->bOrientRotationToMovement = false;
    
    // UseControllerDesiredRotation을 true로 설정
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    
    // 활 당기는 소리 재생
    if (BowDrawSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BowDrawSound, GetActorLocation());
    }

    // 활 모드: 화살 장전 시작
    StartLoadingArrow();
    
    UE_LOG(LogTemp, Log, TEXT("Aiming started"));
}

// 에이밍 종료
void ARLCharacterPlayer::StopAiming()
{
    if (!bIsAiming)
    {
        return;
    }

    bIsAiming = false;
    
    // 에이밍 상태 변화 델리게이트 브로드캐스트
    AimingStateChanged.Broadcast(false);
    
    // 카메라 위치를 일반 모드로 복원 (FollowCamera의 상대적 위치)
    if (FollowCamera)
    {
        FollowCamera->SetRelativeLocation(NormalCameraPosition);
    }
    
    // 일반 상태로 SpringArm 길이 복원
    CameraTargetArmLength = NormalCameraDistance;
    
    // OrientRotationToMovement를 true로 복원
    GetCharacterMovement()->bOrientRotationToMovement = true;
    
    // UseControllerDesiredRotation을 false로 복원
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    
    UE_LOG(LogTemp, Log, TEXT("Aiming stopped"));
}

// 폼 체인지 (검/활 전환)
void ARLCharacterPlayer::ChangeForm()
{
    // 에이밍 중이거나 구르기 중일 때는 폼 체인지 불가
    if (bIsAiming || bIsRolling)
    {
        return;
    }

    // 폼 전환
    bIsSword = !bIsSword;
    
    // 활 메시 가시성 제어
    if (BowMeshComponent)
    {
        BowMeshComponent->SetVisibility(!bIsSword); // 검 모드이면 숨김, 활 모드이면 표시
    }
    
    // WeaponMeshComponent(검) 가시성 제어
    if (WeaponMeshComponent)
    {
        WeaponMeshComponent->SetVisibility(bIsSword); // 검 모드이면 표시, 활 모드이면 숨김
    }
    
    // 화살 갯수 UI 표시/숨김 제어
    if (PlayerUI)
    {
        if (!bIsSword) // 활 모드
        {
            PlayerUI->ShowArrowCount();
            PlayerUI->ShowArrowIcon();
            // 화살 갯수 업데이트
            ArrowCountChanged.Broadcast(CurrentArrowCount, MaxArrowCount);
        }
        else // 검 모드
        {
            PlayerUI->HideArrowCount();
            PlayerUI->HideArrowIcon();
        }
    }
    
    // 에이밍 상태가 활성화되어 있다면 비활성화
    if (bIsAiming && bIsSword)
    {
        StopAiming();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Form changed to: %s"), bIsSword ? TEXT("Sword") : TEXT("Bow"));
}

// 공격 버튼 눌렀을 때
void ARLCharacterPlayer::OnAttackPressed()
{
    if (bIsSword)
    {
        // 검 모드: 기존 공격
        Attack();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("BowActionPressed"));
        if (!bIsLoadingArrow)
        {
            // 활 모드: 화살 장전 시작
            StartLoadingArrow();
        }
    }
}

// 공격 버튼 뗐을 때
void ARLCharacterPlayer::OnAttackReleased()
{
    if (!bIsSword && bIsArrowLoaded)
    {
        // 활 모드이고 화살이 장전된 상태: 화살 발사
        FireArrow();
    }
    else if (!bIsSword && bIsChargingArrow)
    {
        // 차징 중이었다면 차징 중지
        StopChargingArrow();
    }
}

void ARLCharacterPlayer::CallAttackCollision()
{
    ARLCharacterBase::CallAttackCollision();
}

// 화살 장전 시작
void ARLCharacterPlayer::StartLoadingArrow()
{
    if (bIsLoadingArrow || bIsArrowLoaded || !bIsAiming)
    {
        return; // 이미 장전 중이거나 장전된 상태
    }
    
    // 화살 갯수 확인
    if (CurrentArrowCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No arrows remaining!"));
        return; // 화살이 없으면 장전 불가
    }

    // 활 시위를 당기는 몽타주 실행
    if (BowDrawMontage && AnimInstance)
    {
        AnimInstance->Montage_Play(BowDrawMontage);
        UE_LOG(LogTemp, Log, TEXT("Bow draw montage started"));
    }

    bIsLoadingArrow = true;
    bIsChargingArrow = true;
    CurrentChargeTime = 0.0f;
    
    LoadArrowToSocket();
    
    UE_LOG(LogTemp, Log, TEXT("Arrow loading and charging started"));
}

void ARLCharacterPlayer::StopChargingArrow()
{
    if (bIsChargingArrow)
    {
        bIsChargingArrow = false;
        UE_LOG(LogTemp, Log, TEXT("Arrow charging stopped. Final charge time: %f"), CurrentChargeTime);
    }
}

// 화살을 소켓에 장전
void ARLCharacterPlayer::LoadArrowToSocket()
{
    if (!ArrowClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArrowClass is not set"));
        bIsLoadingArrow = false;
        return;
    }

    // 풀에서 화살 가져오기
    LoadedArrow = ArrowPool->GetArrowFromPool();
    if (!LoadedArrow)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get arrow from pool"));
        bIsLoadingArrow = false;
        return;
    }

    // 화살을 hand_rArrowSocket에 부착
    LoadedArrow->SetActorHiddenInGame(false);
    LoadedArrow->AttachToSocket(GetMesh(), TEXT("hand_rArrowSocket"));
    
    bIsLoadingArrow = false;
    bIsArrowLoaded = true;
    
    UE_LOG(LogTemp, Log, TEXT("Arrow loaded to socket"));
}

// 화살 발사
void ARLCharacterPlayer::FireArrow()
{
    if (!bIsArrowLoaded || !LoadedArrow)
    {
        return;
    }

    // 발사 위치 및 방향 설정
    FVector FireLocation = BowMeshComponent ? BowMeshComponent->GetComponentLocation() : GetActorLocation();
    
    // BowMeshComponent의 ForwardVector로 발사 방향 설정
    FVector FireDirection = BowMeshComponent ? BowMeshComponent->GetForwardVector() : GetActorForwardVector();

    // 소켓에서 분리
    LoadedArrow->DetachFromSocket();
    
    // 차징된 속도 계산
    float ArrowSpeed = CalculateArrowSpeed();
    
    // 차징 종료
    StopChargingArrow();
    
    // 화살 초기화 및 발사 (카메라 기준으로 수정)
    LoadedArrow->InitializeArrow(FireLocation, PlayerController, 50, ArrowSpeed);
    
    // 화살 갯수 감소
    CurrentArrowCount--;
    UE_LOG(LogTemp, Log, TEXT("Arrow fired! Remaining arrows: %d"), CurrentArrowCount);
    
    // 화살 갯수 UI 업데이트
    ArrowCountChanged.Broadcast(CurrentArrowCount, MaxArrowCount);

    // 화살 자동 반환을 위한 타이머 설정 (10초 후 풀에 반환)
    ARLArrow* FiredArrow = LoadedArrow;
    FTimerHandle ArrowReturnHandle;
    GetWorld()->GetTimerManager().SetTimer(ArrowReturnHandle, [this, FiredArrow]()
    {
        if (FiredArrow && IsValid(FiredArrow))
        {
            ArrowPool->ReturnArrowToPool(FiredArrow);
        }
    }, 10.0f, false);

    // 상태 초기화
    LoadedArrow = nullptr;
    bIsArrowLoaded = false;
    
    UE_LOG(LogTemp, Log, TEXT("Arrow fired with speed: %f"), ArrowSpeed);
}

// 차징 시스템 업데이트
void ARLCharacterPlayer::UpdateCharging(float DeltaTime)
{
    if (bIsChargingArrow)
    {
        CurrentChargeTime += DeltaTime;
        
        // 최대 차징 시간 제한
        if (CurrentChargeTime >= MaxChargeTime)
        {
            CurrentChargeTime = MaxChargeTime;
            UE_LOG(LogTemp, Log, TEXT("Arrow fully charged!"));
        }
    }
}

// 차징 시간에 따른 화살 속도 계산
float ARLCharacterPlayer::CalculateArrowSpeed() const
{
    if (!bIsChargingArrow && CurrentChargeTime <= 0.0f)
    {
        return MinArrowSpeed; // 차징하지 않았으면 최소 속도
    }
    
    // 차징 비율 계산 (0.0 ~ 1.0)
    float ChargeRatio = FMath::Clamp(CurrentChargeTime / MaxChargeTime, 0.0f, 1.0f);
    
    // 선형 보간으로 속도 계산
    float CalculatedSpeed = FMath::Lerp(MinArrowSpeed, MaxArrowSpeed, ChargeRatio);
    
    UE_LOG(LogTemp, Log, TEXT("Charge ratio: %f, Arrow speed: %f"), ChargeRatio, CalculatedSpeed);
    
    return CalculatedSpeed;
}









