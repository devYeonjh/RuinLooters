// RLCharacterBase.cpp

#include "RLCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Character/RLCharacterEnemy.h"
#include "Character/RLCharacterEnemyDragon.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameInstance/RLGameInstance.h"
#include "RLGliderComponent.h"
#include "Engine/DamageEvents.h"
#include "Particles/ParticleSystem.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

ARLCharacterBase::ARLCharacterBase() : WeaponRowName(TEXT("First WeaponRowName Text"))
{
    // 기본 스탯 초기화 (블루프린트에서 덮어쓰기 가능)
    MaxHp = 100.0f;
    CurrentHp = MaxHp;
    AttackDamage = 0;
    Defence = 5;
    Range = 0;
    MontageSpeed = 0.0f;

    // 1) WeaponMeshComponent 생성
    WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    // 2) 캐릭터 손 소켓에 부착하기 ("hand_rSocket" 등 스켈레톤 이름에 따라 다름)
    WeaponMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_rSwordSocket"));
    // 3) 초기에는 메시 숨김
    WeaponMeshComponent->SetSkeletalMesh(nullptr);
    WeaponMeshComponent->SetCastShadow(false);

}

void ARLCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 월드, 게임인스턴스 찾기
    World = GetWorld();
    GameInstance = Cast<URLGameInstance>(UGameplayStatics::GetGameInstance(World));
    ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    AnimInstance = GetMesh()->GetAnimInstance();
}

float ARLCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    // 방어력만큼 데미지 감소
    float DamageApplied = FMath::Max(1.0f, DamageAmount - Defence);
    CurrentHp -= DamageApplied;

    if (CurrentHp < 0)
    {
        CurrentHp = 0;
    }

    // 타격 파티클 이펙트 생성
    if (HitParticleTemplate)
    {
        FVector HitLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f); // 캐릭터 중앙 위치
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticleTemplate, HitLocation);
    }

    if (CurrentHp <= 0)
    {
        Die();
    }

    if (HitSound && CurrentHp > 0)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }

    return DamageApplied;
}

void ARLCharacterBase::TakeCharacterHeal(int32 RecieveHealAmount)
{
    UE_LOG(LogTemp, Warning, TEXT("PreviousHp: %.1f."), CurrentHp);

    float TotalHp = RecieveHealAmount + CurrentHp;

    CurrentHp = FMath::Clamp(TotalHp, 0.0f, MaxHp);

    UE_LOG(LogTemp, Warning, TEXT("CurrentHp: %.1f."), CurrentHp);
}

void ARLCharacterBase::Die()
{
    if (DieSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
    }

    if (AnimInstance && DieMontage)
    {        
        // DieMontage 재생
        AnimInstance->StopAllMontages(0.0);
        AnimInstance->Montage_Play(DieMontage);
    }

    SetActorEnableCollision(false);

    CharacterDie.Broadcast();
}

void ARLCharacterBase::CallAttackCollision()
{
    // 캐릭터 앞에 캡슐 콜리전 생성하여 범위 내 적들에게 데미지 적용
    FVector ForwardVector = GetActorForwardVector();
    FVector StartLocation = GetActorLocation() + ForwardVector * 100.0f;
    FVector CapsuleCenter = StartLocation + ForwardVector * (Range * 0.5f);
    
    // 캡슐 콜리전 파라미터 설정 (둘레는 고정, 길이는 Range 값 사용)
    float CapsuleRadius = 30.0f; // 고정된 둘레
    float CapsuleHalfHeight = Range * 0.5f; // Range 값을 길이로 사용
    
    // 콜리전 쿼리 파라미터 설정
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 자신은 제외
    
    // 캐릭터의 로컬 Y축(Right Vector)을 기준으로 90도 회전된 쿼터니언 생성
    FVector LocalRightVector = GetActorRightVector();
    FQuat CapsuleRotation = FQuat(LocalRightVector, FMath::DegreesToRadians(90.0f));
    
    // 캡슐 모양으로 충돌 검사
    TArray<FHitResult> HitResults;
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        CapsuleCenter + ForwardVector * CapsuleHalfHeight,
        CapsuleRotation,
        ECC_Pawn,
        FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
        QueryParams
    );
    
    if (bHit)
    {
        // 중복 데미지 방지를 위한 액터 추적
        TArray<AActor*> DamagedActors;
        
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor || DamagedActors.Contains(HitActor))
                continue;
                
            // ARLCharacterBase 타입 체크
            ARLCharacterBase* HitCharacter = Cast<ARLCharacterBase>(HitActor);
            if (HitCharacter && HitCharacter != this)
            {
                FDamageEvent DamageEvent;
                HitCharacter->TakeDamage((float)AttackDamage, DamageEvent, nullptr, this);
                DamagedActors.Add(HitActor);
                continue;
            }
            
            // ARLCharacterEnemyDragon 타입 체크
            ARLCharacterEnemyDragon* HitDragon = Cast<ARLCharacterEnemyDragon>(HitActor);
            if (HitDragon)
            {
                FDamageEvent DamageEvent;
                HitDragon->TakeDamage((float)AttackDamage, DamageEvent, nullptr, this);
                DamagedActors.Add(HitActor);
            }
        }
    }
}

void ARLCharacterBase::PlayAttackSound()
{
    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }
}

void ARLCharacterBase::ChangeWeapon(FWeaponTableRow* ChangeWeapon)
{
    // 무기 능력치 적용, 스켈레탈 메시 설정
    ApplyWeaponAbility(ChangeWeapon);
    WeaponMeshComponent->SetSkeletalMesh(ChangeWeapon->SkeletalMesh);

    // 무기별 회전각도 조정 설정
    if (ChangeWeapon->WeaponIndex == 1 || ChangeWeapon->WeaponIndex == 3)
    {
        WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    }
    //else if (ChangeWeapon->WeaponIndex == 9 || ChangeWeapon->WeaponIndex == 10)
    //{
    //    WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    //}


    // 데이터 테이블의 무기 정보 저장
    RowWeapon = ChangeWeapon;
}

void ARLCharacterBase::ApplyWeaponAbility(FWeaponTableRow* ApplyWeapon)
{
    AttackDamage = ApplyWeapon->Damage;
    MontageSpeed = ApplyWeapon->MontageSpeed;
    Range = ApplyWeapon->Range;
}

void ARLCharacterBase::Attack()
{
    ProcessComboCommand();
}

void ARLCharacterBase::ProcessComboCommand()
{
    UE_LOG(LogTemp, Warning, TEXT("CurrentCombo is : %d"), CurrentCombo);
    if (CurrentCombo == 0)
    {
        ComboActionBegin();
        return;
    }

    if (!ComboTimerHandle.IsValid())
    {
        HasNextComboCommand = false;
    }
    else
    {
        HasNextComboCommand = true;
        
    }
}

// --- 롤(구르기) Tick 함수 구현 ---
void ARLCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsRolling)
    {
        // 구르기 중에는 구르기 방향으로 자동 이동
        AddMovementInput(RollDirection, 1.0f);
    }
}

// --- 롤(구르기) 시작 ---
void ARLCharacterBase::StartRoll()
{
    ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(this);
    if (Player && Player->GliderComponent && Player->GliderComponent->IsGliderActive())
        return;
    if (bIsRolling || !RollMontage) return;
    if (GetCharacterMovement()->IsFalling()) return;
    bIsRolling = true;
    FVector InputDir = GetLastMovementInputVector();
    RollDirection = InputDir.IsNearlyZero() ? GetActorForwardVector() : InputDir.GetSafeNormal();

    // 구르기 방향을 바라보도록 캐릭터 회전
    FRotator TargetRot = RollDirection.Rotation();
    SetActorRotation(TargetRot);

    // 구르기 전 속도 저장 & 1.5배로 설정
    OriginalMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed * 1.5f;

    // 플레이어 입력 차단
    DisablePlayerInput();

    // 롤 몽타주 재생 및 종료 델리게이트 설정
    AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(RollMontage);
        
        // 몽타주 종료 시 EndRoll() 호출되도록 델리게이트 설정
        FOnMontageEnded RollEndDelegate;
        RollEndDelegate.BindUObject(this, &ARLCharacterBase::OnRollMontageEnded);
        AnimInstance->Montage_SetEndDelegate(RollEndDelegate, RollMontage);
    }
    else
    {
        PlayAnimMontage(RollMontage);
    }
}

// --- 롤 몽타주 종료 콜백 ---
void ARLCharacterBase::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 롤 몽타주인지 확인 후 EndRoll() 호출
    if (Montage == RollMontage)
    {
        bIsRolling = false;
        // 구르기 끝나면 원래 속도로 복귀
        GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
        
        // 플레이어 입력 복원
        EnablePlayerInput();
    }
}

// --- 무적 시작(애님 노티파이용) ---
void ARLCharacterBase::StartInvincible()
{
    bIsInvincible = true;
}

// --- 무적 종료(애님 노티파이용) ---
void ARLCharacterBase::EndInvincible()
{
    bIsInvincible = false;
}

void ARLCharacterBase::Move(const FInputActionValue& Value)
{
    Super::Move(Value);
}

void ARLCharacterBase::ComboActionBegin()
{
    // Combo Status
    CurrentCombo = 1;

    // Movement Setting
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

    // Animation Setting
    AttackSpeedRate = 1.5f;
    AnimInstance = GetMesh()->GetAnimInstance();
    AnimInstance->Montage_Play(ComboActionMontage, AttackSpeedRate);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &ARLCharacterBase::ComboActionEnd);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboActionMontage);

    ComboTimerHandle.Invalidate();
    SetComboCheckTimer();
}

void ARLCharacterBase::ComboActionEnd(UAnimMontage* Montage, bool bInterrupted)
{
    ensure(CurrentCombo != 0);
    CurrentCombo = 0;
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ARLCharacterBase::SetComboCheckTimer()
{
    int32 ComboIndex = CurrentCombo - 1;
    ensure(ComboActionData->EffectiveFrameCount.IsValidIndex(ComboIndex));

    float ComboEffectiveTime = (ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate) / AttackSpeedRate;
    if (ComboEffectiveTime > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &ARLCharacterBase::ComboCheck, ComboEffectiveTime, false);
    }
}

void ARLCharacterBase::ComboCheck()
{
     ComboTimerHandle.Invalidate();
    if (HasNextComboCommand)
    {
        AnimInstance = GetMesh()->GetAnimInstance();

        CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, ComboActionData->MaxComboCount);
        FName NextSection = *FString::Printf(TEXT("%s%d"), *ComboActionData->MontageSectionNamePrefix, CurrentCombo);
        AnimInstance->Montage_JumpToSection(NextSection, ComboActionMontage);
        SetComboCheckTimer();
        HasNextComboCommand = false;
    }
}
