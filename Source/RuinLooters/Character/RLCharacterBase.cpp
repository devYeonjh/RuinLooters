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
    // 콤보 데이터 에셋이 없으면 기본 공격
    if (!ComboDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("ComboDataAsset is not set. Using default attack."));
        
        // 기본 공격 로직
        if (!bIsCanAttack || GetCharacterMovement()->IsFalling())
        {
            return;
        }
        
        bIsCanAttack = false;
        if (CurrentMontage && AnimInstance)
        {
            AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);
        }
        return;
    }

    // 점프 중이거나 공중에 떠있으면 공격 불가
    if (GetCharacterMovement()->IsFalling())
    {
        UE_LOG(LogTemp, Warning, TEXT("Can't Attack - Character is falling"));
        return;
    }

    // 현재 콤보 진행 중인 경우
    if (bIsAttacking)
    {
        // 다음 콤보 입력 가능한 상태라면 입력 저장
        if (bCanNextCombo && CurrentComboStep < ComboDataAsset->MaxComboCount)
        {
            bComboInput = true;
            UE_LOG(LogTemp, Log, TEXT("Combo Input Registered - Next combo will be: %d"), CurrentComboStep + 1);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Can't register combo input - bCanNextCombo: %s, CurrentComboStep: %d/%d"), 
                   bCanNextCombo ? TEXT("true") : TEXT("false"), CurrentComboStep, ComboDataAsset->MaxComboCount);
        }
        return;
    }

    // 새로운 콤보 시작
    if (bIsCanAttack)
    {
        StartComboSequence();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Can't Attack - bIsCanAttack is false"));
    }
}

void ARLCharacterBase::StartComboSequence()
{
    if (!ComboDataAsset || !CurrentMontage || !AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartComboSequence failed - Missing required components"));
        return;
    }

    // 콤보 시스템 초기화
    CurrentComboStep = 1;
    bIsAttacking = true;
    bIsCanAttack = false;
    bComboInput = false;
    bCanNextCombo = false;

    // 첫 번째 콤보 재생
    ProcessNextCombo();

    UE_LOG(LogTemp, Log, TEXT("Combo Sequence Started - Step 1/%d"), ComboDataAsset->MaxComboCount);
}

void ARLCharacterBase::ProcessNextCombo()
{
    if (!ComboDataAsset || !CurrentMontage || !AnimInstance)
    {
        ResetCombo();
        return;
    }

    // 콤보 단계 유효성 검사
    if (CurrentComboStep < 1 || CurrentComboStep > ComboDataAsset->MaxComboCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid combo step: %d"), CurrentComboStep);
        ResetCombo();
        return;
    }

    // 몽타주 섹션 이름 생성 (예: "ComboAttack1", "ComboAttack2", "ComboAttack3")
    FString SectionName = ComboDataAsset->MontageSectionNamePrefix + FString::FromInt(CurrentComboStep);
    FName SectionFName = FName(*SectionName);

    // 몽타주 재생
    if (AnimInstance->Montage_IsPlaying(CurrentMontage))
    {
        AnimInstance->Montage_JumpToSection(SectionFName, CurrentMontage);
    }
    else
    {
        AnimInstance->Montage_Play(CurrentMontage, AttackSpeed);
        AnimInstance->Montage_JumpToSection(SectionFName, CurrentMontage);
    }

    // 몽타주 끝남 이벤트 바인딩
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &ARLCharacterBase::OnMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);

    // 다음 콤보 입력 타이밍 설정
    SetupComboInputTiming();

    UE_LOG(LogTemp, Log, TEXT("Playing Combo Step %d/%d - Section: %s"), 
           CurrentComboStep, ComboDataAsset->MaxComboCount, *SectionName);
}

void ARLCharacterBase::SetupComboInputTiming()
{
    if (!ComboDataAsset || ComboDataAsset->EffectiveFrameCount.Num() < CurrentComboStep)
    {
        return;
    }

    // 현재 콤보 단계의 효과적인 프레임 수 가져오기 (배열 인덱스는 0부터 시작)
    float EffectiveFrame = ComboDataAsset->EffectiveFrameCount[CurrentComboStep - 1];
    
    // -1.0은 콤보 입력 불가를 의미 (마지막 콤보)
    if (EffectiveFrame < 0.0f)
    {
        bCanNextCombo = false;
        UE_LOG(LogTemp, Log, TEXT("Final combo step - No next combo available"));
        return;
    }

    // 프레임을 초 단위로 변환
    float InputWindowTime = EffectiveFrame / ComboDataAsset->FrameRate;
    
    // 다음 콤보 입력 가능 시간 설정
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("CheckComboInput"));
    
    GetWorldTimerManager().SetTimer(ComboTimerHandle, TimerDelegate, InputWindowTime, false);
    
    // 즉시 입력 가능 상태로 설정
    bCanNextCombo = true;
    
    UE_LOG(LogTemp, Log, TEXT("Combo input window opened for %.2f seconds (Frame: %.1f)"), 
           InputWindowTime, EffectiveFrame);
}

void ARLCharacterBase::CheckComboInput()
{
    if (!bIsAttacking)
    {
        return;
    }

    // 콤보 입력이 있고 다음 콤보가 가능한 상태
    if (bComboInput && CurrentComboStep < ComboDataAsset->MaxComboCount)
    {
        // 다음 콤보 단계로 진행
        CurrentComboStep++;
        bComboInput = false;
        bCanNextCombo = false;
        
        // 다음 콤보 실행
        ProcessNextCombo();
        
        UE_LOG(LogTemp, Log, TEXT("Combo continued to step %d"), CurrentComboStep);
    }
    else
    {
        // 콤보 입력이 없었거나 마지막 콤보에 도달
        bCanNextCombo = false;
        UE_LOG(LogTemp, Log, TEXT("Combo input window closed - No input detected"));
    }
}

void ARLCharacterBase::ResetCombo()
{
    CurrentComboStep = 0;
    bIsAttacking = false;
    bIsCanAttack = true;
    bComboInput = false;
    bCanNextCombo = false;
    
    // 타이머 정리
    GetWorldTimerManager().ClearTimer(ComboTimerHandle);
    
    UE_LOG(LogTemp, Log, TEXT("Combo system reset"));
}

void ARLCharacterBase::CallAttackCollision()
{
}

void ARLCharacterBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Log, TEXT("Montage ended - bInterrupted: %s"), bInterrupted ? TEXT("true") : TEXT("false"));
    
    // 콤보 시스템 리셋
    ResetCombo();
    
    // 공격 끝나면 이동 가능
    //GetCharacterMovement()->SetMovementMode(MOVE_Walking);
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







