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
    AnimInstance = GetMesh()->GetAnimInstance();

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

    if (AnimInstance && DieMontage)
    {        
        // DieMontage 재생
        AnimInstance->StopAllMontages(0.0);
        AnimInstance->Montage_Play(DieMontage);
    }

    SetActorEnableCollision(false);

    CharacterDie.Broadcast();
}

void ARLCharacterBase::Attack()
{
    // 점프 중이거나 공중에 떠있으면 공격 불가
    if (!bIsCanAttack || GetCharacterMovement()->IsFalling() || !bCanNextCombo)
    {
        UE_LOG(LogTemp, Warning, TEXT("Can't Attack"));
        return;
    }

    // Idle 상태에서만 1타 시작
    bComboInput = false;
    bIsCanAttack = false;
    CurrentComboStep = 1;
    PlayComboMontage(CurrentComboStep);
    // 공격 시작 시 이동 불가
    //GetCharacterMovement()->DisableMovement();
    if (AnimInstance && CurrentMontage)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &ARLCharacterBase::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
    }
}

void ARLCharacterBase::CallAttackCollision()
{
}

void ARLCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage ended. Timer will stop."));
    // 공격 끝나면 이동 가능
    //GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    bIsCanAttack = true;
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

void ARLCharacterBase::PlayComboMontage(int32 ComboStep)
{
    if (!CurrentMontage || !AnimInstance) return;
    if (ComboStep > 0 && ComboStep <= CurrentMontage->CompositeSections.Num())
    {
        FName SectionName = CurrentMontage->CompositeSections[ComboStep - 1].SectionName;
        if (AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
        }
        else
        {
            AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);
            AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
        }
    }
    CurrentComboStep = ComboStep;
}







