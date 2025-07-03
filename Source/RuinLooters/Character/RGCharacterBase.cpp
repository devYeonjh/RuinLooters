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
    // �⺻ ���� �ʱ�ȭ (���Ͻô� ������ ����)
    MaxHp = 100;
    CurrentHp = MaxHp;
    AttackDamage = 0;
    Defence = 5;
    Range = 0;
    AttackSpeed = 0.0f;
    bIsCanAttack = true;


    // 1) WeaponMeshComponent ����
    WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    // 2) ĳ���� �� ���Ͽ� ���̱� ("hand_rSocket" �� ���� �̸� ���� ���� �̸�)
    WeaponMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_rSwordSocket"));
    // 3) �ʱ⿡�� �޽� ����
    WeaponMeshComponent->SetSkeletalMesh(nullptr);
    WeaponMeshComponent->SetCastShadow(false);

}

void ARGCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // ����, ���Ӹ�� ã��
    World = GetWorld();
    GameInstance = Cast<URGGameInstance>(UGameplayStatics::GetGameInstance(World));
    ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ARGCharacterBase::TakeCharacterDamage(int32 RecieveDamage)
{
    // ���¸�ŭ ������ �氨
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

        // ��Ÿ�� ��ü & ���
        AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            // ��Ÿ�� ���
            AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);

            // ������ �� ȣ��� ��������Ʈ ���ε�
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ARGCharacterBase::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
        }
    }
}

void ARGCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage ended. Timer will stop."));

    // ���� ����
    bIsCanAttack = true;
}

void ARGCharacterBase::SwordAttackLineTrace()
{
    // ���濡 ���� Ʈ���̽��� �ɾ� ���� ĳ���Ϳ� ������ ���� ����
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * Range;  // ��Ÿ� ����

    FHitResult Hit;     // Ʈ���̽�, �浹�� ����� ��� ����ü
    FCollisionQueryParams Params;   // ���� Ʈ���̽�, ����, �������� ��� ���� �����ϴ� ����ü
    Params.AddIgnoredActor(this);   // ������ ���� ���� (�ڽ� ����)

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
        // ������ ����
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
    // ���� �ɷ�ġ ����, ���̷�Ż �޽� ����
    ApplyWeaponAbility(ChangeWeapon);
    WeaponMeshComponent->SetSkeletalMesh(ChangeWeapon->SkeletalMesh);

    // ���⺰ ȸ������ ���� ����
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

    // ������ ���̺��� ���� �� ����
    RowWeapon = ChangeWeapon;
}

void ARGCharacterBase::ApplyWeaponAbility(FWeaponTableRow* ApplyWeapon)
{
    AttackDamage = ApplyWeapon->Damage;
    AttackSpeed = ApplyWeapon->AttackSpeed;
    Range = ApplyWeapon->Range;
}
