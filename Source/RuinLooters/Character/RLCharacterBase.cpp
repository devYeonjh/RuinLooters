// RLCharacterBase.cpp

#include "RLCharacterBase.h"
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
    if (bIsCanAttack)
    {
        bIsCanAttack = false;

        // 몬타주 객체 & 재생
        AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            // 몬타주 재생
            AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);

            // 종료될 때 호출할 델리게이트 바인딩
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ARLCharacterBase::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);
        }
    }
}

void ARLCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Warning, TEXT("Montage ended. Timer will stop."));

    // 공격 가능
    bIsCanAttack = true;
}

void ARLCharacterBase::SwordAttackLineTrace()
{
    // ���濡 ���� Ʈ���̽��� �ɾ� ���� ĳ���Ϳ� ������ ���� ����
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * Range;  // ��Ÿ�?����

    FHitResult Hit;     // Ʈ���̽�, �浹�� �����?���?����ü
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



