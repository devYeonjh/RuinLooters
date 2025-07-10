// RLCharacterBase.cpp

#include "RLCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Character/RLCharacterEnemy.h"
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

ARLCharacterBase::ARLCharacterBase() : WeaponRowName(TEXT("First WeaponRowName Text"))
{
    // 기본 스탯 초기화 (블루프린트에서 덮어쓰기 가능)
    MaxHp = 100;
    CurrentHp = MaxHp;
    AttackDamage = 0;
    Defence = 5;
    Range = 0;
    AttackSpeed = 0.0f;

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

void ARLCharacterBase::TakeCharacterDamage(int32 RecieveDamage)
{
    // 방어력만큼 데미지 감소
    int32 DamageApplied = FMath::Max(1, RecieveDamage - Defence);
    CurrentHp -= DamageApplied;

    if (CurrentHp < 0)
    {
        CurrentHp = 0;
    }

    if (CurrentHp == 0)
    {
        Die();
    }

    if (HitSound && CurrentHp > 0)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }

}

void ARLCharacterBase::TakeCharacterHeal(int32 RecieveHealAmount)
{
    UE_LOG(LogTemp, Warning, TEXT("PreviousHp: %d."), CurrentHp);

    int32 TotalHp = RecieveHealAmount + CurrentHp;

    CurrentHp = FMath::Clamp(TotalHp, 0, MaxHp);

    UE_LOG(LogTemp, Warning, TEXT("CurrentHp: %d."), CurrentHp);
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
}

void ARLCharacterBase::SwordAttackLineTrace()
{
    // 전방에 대한 트레이스를 걸어서 적이 캐릭터에 닿으면 데미지 적용
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * Range;  // 공격 범위

    FHitResult Hit;     // 트레이스, 충돌시 충돌정보를 담는 구조체
    FCollisionQueryParams Params;   // 충돌 쿼리, 충돌 검사, 충돌 검사 후 처리 함수 설정
    Params.AddIgnoredActor(this);   // 본인은 무시 설정 (자신 제외)

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Pawn,
        Params
    );

    UE_LOG(LogTemp, Warning, TEXT("Attack Executed"));

    if (bHit)
    {
        // 히트시 처리
        ARLCharacterBase* HitChar = Cast<ARLCharacterBase>(Hit.GetActor());
        if (HitChar)
        {
            HitChar->TakeCharacterDamage(AttackDamage);
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
    if (ChangeWeapon->WeaponIndex == 0)
    {
        WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    }
    //else if (ChangeWeapon->WeaponIndex == 9 || ChangeWeapon->WeaponIndex == 10)
    //{
    //    WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    //}
    else
    {
        WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    }

    // 데이터 테이블의 무기 정보 저장
    RowWeapon = ChangeWeapon;
}

void ARLCharacterBase::ApplyWeaponAbility(FWeaponTableRow* ApplyWeapon)
{
    AttackDamage = ApplyWeapon->Damage;
    AttackSpeed = ApplyWeapon->AttackSpeed;
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

    PlayAnimMontage(RollMontage);
}

// --- 롤(구르기) 종료 ---
void ARLCharacterBase::EndRoll()
{
    bIsRolling = false;
    // 구르기 끝나면 원래 속도로 복귀
    GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
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
