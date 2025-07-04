// RGCharacterBase.cpp

#include "RGCharacterBase.h"
#include "Character/RGCharacterPlayer.h"
#include "Character/RGCharacterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameInstance/RGGameInstance.h"

ARGCharacterBase::ARGCharacterBase() : WeaponRowName(TEXT("First WeaponRowName Text"))
{
    // 기본 초기화 (초기 값 설정)
    MaxHp = 100;
    CurrentHp = MaxHp;
    AttackDamage = 0;
    Defence = 5;
    Range = 0;
    AttackSpeed = 0.0f;
    bIsCanAttack = true;


    // 1) WeaponMeshComponent 설정
    WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    // 2) 특정 손에 장비 장착 ("hand_rSocket" 위치에 장비 위치 이름 설정)
    WeaponMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_rSwordSocket"));
    // 3) 기본 애니메이션 설정
    WeaponMeshComponent->SetSkeletalMesh(nullptr);
    WeaponMeshComponent->SetCastShadow(false);

}

void ARGCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 찾기, 자식 찾기
    World = GetWorld();
    GameInstance = Cast<URGGameInstance>(UGameplayStatics::GetGameInstance(World));
    ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    // 콤보 최대 단계 자동 설정
    if (CurrentMontage)
    {
        ComboMaxStep = CurrentMontage->CompositeSections.Num();
    }
}

void ARGCharacterBase::TakeCharacterDamage(int32 RecieveDamage)
{
    // 최대 피해량 계산
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

void ARGCharacterBase::TakeCharacterHeal(int32 RecieveHealAmount)
{
    UE_LOG(LogTemp, Warning, TEXT("PreviousHp: %d."), CurrentHp);

    int32 TotalHp = RecieveHealAmount + CurrentHp;

    CurrentHp = FMath::Clamp(TotalHp, 0, MaxHp);

    UE_LOG(LogTemp, Warning, TEXT("CurrentHp: %d."), CurrentHp);
}

void ARGCharacterBase::Die()
{
    if (DieSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
    }

    GetMesh()->GetAnimInstance()->Montage_Play(DieMontage);

    SetActorEnableCollision(false);

    CharacterDie.Broadcast();
}

void ARGCharacterBase::Attack()
{
    if (bIsCanAttack)
    {
        bIsCanAttack = false;
        AnimInstance = GetMesh()->GetAnimInstance();
        CurrentComboStep = 1;
        PlayComboMontage(CurrentComboStep);
        // 델리게이트 설정 (몽타주 종료 시 콤보 상태 초기화)
        if (AnimInstance && CurrentMontage)
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ARGCharacterBase::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
        }
    }
    else if (bCanNextCombo)
    {
        bComboInput = true;
    }
}

void ARGCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage ended. Timer will stop."));

    // 충돌 후 처리
    bIsCanAttack = true;
}

void ARGCharacterBase::SwordAttackLineTrace()
{
    // 충돌 후 처리를 위해 충돌 검사 함수 구현
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * Range;  // 충돌 위치

    FHitResult Hit;     // 충돌 결과, 충돌 검사 결과를 저장할 변수
    FCollisionQueryParams Params;   // 충돌 검사, 무시할 액터, 충돌 검사 결과를 저장할 변수
    Params.AddIgnoredActor(this);   // 자신을 무시 (자식 찾기)

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
        // 충돌된 캐릭터 처리
        ARGCharacterBase* HitChar = Cast<ARGCharacterBase>(Hit.GetActor());
        if (HitChar)
        {
            HitChar->TakeCharacterDamage(AttackDamage);
        }
    }
}

void ARGCharacterBase::PlayAttackSound()
{
    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }
}

void ARGCharacterBase::ChangeWeapon(FWeaponTableRow* ChangeWeapon)
{
    // 장비 적용, 장비 애니메이션 설정
    ApplyWeaponAbility(ChangeWeapon);
    WeaponMeshComponent->SetSkeletalMesh(ChangeWeapon->SkeletalMesh);

    // 장비 위치 설정
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

    // 장비 행동 설정
    RowWeapon = ChangeWeapon;
}

void ARGCharacterBase::ApplyWeaponAbility(FWeaponTableRow* ApplyWeapon)
{
    AttackDamage = ApplyWeapon->Damage;
    AttackSpeed = ApplyWeapon->AttackSpeed;
    Range = ApplyWeapon->Range;
}

void ARGCharacterBase::PlayComboMontage(int32 ComboStep)
{
    if (!CurrentMontage || !AnimInstance) return;
    if (ComboStep > 0 && ComboStep <= CurrentMontage->CompositeSections.Num())
    {
        FName SectionName = CurrentMontage->CompositeSections[ComboStep - 1].SectionName;
        // AttackSpeed를 반영하여 재생
        AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);
        AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
    }
    CurrentComboStep = ComboStep;
}
