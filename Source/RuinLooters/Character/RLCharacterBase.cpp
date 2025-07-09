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
    bIsCanAttack = true;


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

    // 콤보 최대 단계 자동 설정
    if (CurrentMontage)
    {
        ComboMaxStep = CurrentMontage->CompositeSections.Num();
    }
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

    GetMesh()->GetAnimInstance()->Montage_Play(DieMontage);

    SetActorEnableCollision(false);

    CharacterDie.Broadcast();
}

void ARLCharacterBase::Attack()
{
    UE_LOG(LogTemp, Warning, TEXT("[Attack] bIsAttacking: %s, CurrentComboStep: %d"), bIsAttacking ? TEXT("true") : TEXT("false"), CurrentComboStep);
    if (bIsRolling) return;

    ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(this);
    if (Player && Player->GliderComponent && Player->GliderComponent->IsGliderActive())
        return;

    // 공격 중(콤보 중)일 때
    if (bIsAttacking)
    {
        // 마지막 콤보가 아니고, 버퍼가 비어있을 때만 버퍼 세팅
        if (CurrentComboStep < ComboMaxStep && !bComboInputBuffered)
        {
            bComboInputBuffered = true;
        }
        // 공격 중에는 절대 1타로 돌아가지 않음!
        return;
    }

    // 공격 중이 아니고, 공격 가능 상태일 때만 1타 시작
    if (bIsCanAttack && !GetCharacterMovement()->IsFalling())
    {
        bIsCanAttack = false;
        bIsAttacking = true;
        AnimInstance = GetMesh()->GetAnimInstance();
        if (CurrentMontage)
            ComboMaxStep = CurrentMontage->CompositeSections.Num();

        PlayComboMontage(1); // 1타(Combo1) 섹션 재생

        if (AnimInstance && CurrentMontage)
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ARLCharacterBase::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
        }
    }
    // 그 외(공격 불가, 공중 등)는 아무 동작 없음
}

void ARLCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage ended. Timer will stop."));
    bIsCanAttack = true;
    bIsAttacking = false; // 공격 종료
}

void ARLCharacterBase::SwordAttackLineTrace()
{
    // 전방에 대한 트레이스를 걸어서 적이 캐릭터에 닿으면 데미지 적용
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * Range;  // 공격 범위

    FHitResult Hit;     // 트레이스, 충돌시 충돌정보를 담는 구조체
    FCollisionQueryParams Params;   // ���� Ʈ���̽�, ����, �������� ��� ���� �����ϴ� ����ü
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
    if (ChangeWeapon->WeaponIndex == 4 || ChangeWeapon->WeaponIndex == 5)
    {
        WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    }
    else if (ChangeWeapon->WeaponIndex == 9 || ChangeWeapon->WeaponIndex == 10)
    {
        WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    }
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

void ARLCharacterBase::PlayComboMontage(int32 ComboStep)
{
    if (!CurrentMontage || !AnimInstance) return;
    if (ComboStep > 0 && ComboStep <= CurrentMontage->CompositeSections.Num())
    {
        FName SectionName = CurrentMontage->CompositeSections[ComboStep - 1].SectionName;
        UE_LOG(LogTemp, Warning, TEXT("[PlayComboMontage] Step: %d, SectionName: %s"), ComboStep, *SectionName.ToString());
        if (AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
            UE_LOG(LogTemp, Warning, TEXT("[PlayComboMontage] Jumped to section: %s"), *SectionName.ToString());
        }
        else
        {
            AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);
            AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
            UE_LOG(LogTemp, Warning, TEXT("[PlayComboMontage] Montage played and jumped to section: %s"), *SectionName.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayComboMontage] Invalid ComboStep: %d (Max: %d)"), ComboStep, CurrentMontage->CompositeSections.Num());
    }
    CurrentComboStep = ComboStep;
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

