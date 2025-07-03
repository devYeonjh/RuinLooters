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
    // 기본 스탯 초기화 (원하시는 값으로 조정)
    MaxHp = 100;
    CurrentHp = MaxHp;
    AttackDamage = 0;
    Defence = 5;
    Range = 0;
    AttackSpeed = 0.0f;
    bIsCanAttack = true;


    // 1) WeaponMeshComponent 생성
    WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    // 2) 캐릭터 손 소켓에 붙이기 ("hand_rSocket" 은 본에 미리 만든 소켓 이름)
    WeaponMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_rSwordSocket"));
    // 3) 초기에는 메시 없음
    WeaponMeshComponent->SetSkeletalMesh(nullptr);
    WeaponMeshComponent->SetCastShadow(false);

}

void ARGCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 월드, 게임모드 찾기
    World = GetWorld();
    GameInstance = Cast<URGGameInstance>(UGameplayStatics::GetGameInstance(World));
    ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ARGCharacterBase::TakeCharacterDamage(int32 RecieveDamage)
{
    // 방어력만큼 데미지 경감
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

        // 몽타주 교체 & 재생
        AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            // 몽타주 재생
            AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);

            // 끝났을 때 호출될 델리게이트 바인딩
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ARGCharacterBase::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
        }
    }
}

void ARGCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage ended. Timer will stop."));

    // 공격 가능
    bIsCanAttack = true;
}

void ARGCharacterBase::SwordAttackLineTrace()
{
    // 전방에 라인 트레이스를 걸어 맞은 캐릭터에 데미지 적용 예시
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * Range;  // 사거리 단위

    FHitResult Hit;     // 트레이스, 충돌의 결과를 담는 구조체
    FCollisionQueryParams Params;   // 라인 트레이스, 스윕, 오버랩을 어떻게 쏠지 설정하는 구조체
    Params.AddIgnoredActor(this);   // 무시할 액터 선택 (자신 무시)

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
        // 데미지 적용
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
    // 무기 능력치 적용, 스켈레탈 메시 적용
    ApplyWeaponAbility(ChangeWeapon);
    WeaponMeshComponent->SetSkeletalMesh(ChangeWeapon->SkeletalMesh);

    // 무기별 회전으로 각도 조절
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

    // 데이터 테이블의 무기 열 지정
    RowWeapon = ChangeWeapon;
}

void ARGCharacterBase::ApplyWeaponAbility(FWeaponTableRow* ApplyWeapon)
{
    AttackDamage = ApplyWeapon->Damage;
    AttackSpeed = ApplyWeapon->AttackSpeed;
    Range = ApplyWeapon->Range;
}
