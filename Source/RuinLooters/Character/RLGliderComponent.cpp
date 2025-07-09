#include "RLGliderComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Character/RLCharacterPlayer.h"

URLGliderComponent::URLGliderComponent()
{
    PrimaryComponentTick.bCanEverTick = true; // Tick 활성화
    bIsActive = false;
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    GliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GliderMesh"));
    GliderMesh->SetVisibility(false);
    GliderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GliderAsset(TEXT("/Game/Model/Wing/GliderFBX.GliderFBX"));
    if (GliderAsset.Succeeded())
    {
        GliderMesh->SetStaticMesh(GliderAsset.Object);
    }
}

void URLGliderComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (OwnerCharacter->GetMesh())
        {
            GliderMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("head"));
            GliderMesh->SetRelativeLocation(FVector(-90, 0, 0)); // 머리 위로 미세조정
            GliderMesh->SetRelativeRotation(FRotator(90, 60, -90)); // 예시: Yaw 90도 회전 (필요에 따라 값 조정)
        }
        else
        {
            GliderMesh->AttachToComponent(OwnerCharacter->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            GliderMesh->SetRelativeLocation(FVector(0, 0, 100));
            GliderMesh->SetRelativeRotation(FRotator(90, 90, -90)); // 동일하게 적용
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[GLIDER] BeginPlay: OwnerCharacter is nullptr! GetOwner()=%p"), GetOwner());
    }
}

void URLGliderComponent::SpawnPortalEffect()
{
    if (!PortalEffectClass.IsValid() || !GetWorld() || !GliderMesh) return;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    FVector SpawnLoc = GliderMesh->GetComponentLocation();
    FRotator SpawnRot = GliderMesh->GetComponentRotation() + FRotator(0, 90, 0);
    AActor* Portal = GetWorld()->SpawnActor<AActor>(PortalEffectClass.Get(), SpawnLoc, SpawnRot, Params);
    if (Portal)
    {
        Portal->SetActorScale3D(FVector::ZeroVector);
        ActivePortalEffects.Add(Portal);
    }
}

void URLGliderComponent::ActivateGlider()
{
    if (!bIsActive && MovementComponent && OwnerCharacter)
    {
        bIsActive = true;
        OriginalGravityScale = MovementComponent->GravityScale;
        OriginalAirControl = MovementComponent->AirControl;
        OriginalMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
        MovementComponent->GravityScale = GliderGravityScale;
        MovementComponent->AirControl = GliderAirControl;
        MovementComponent->MaxWalkSpeed = GliderMaxWalkSpeed;
        if (GliderMesh) {
            GliderMesh->SetVisibility(true);
            GliderMesh->SetWorldScale3D(FVector(0,0,0)); // 시작은 0
            GliderTargetScale = FVector(1,1,1); // 목표는 1
            bGliderScaling = true;
        }
        SpawnPortalEffect();
        if (OwnerCharacter)
        {
            ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OwnerCharacter);
            if (Player)
            {
                Player->CameraTargetArmLength = 1200.0f; // 글라이딩 시 더 멀리
            }
        }
        // 글라이드 모드: 엔진 자동 회전 비활성화 (진입 전 상태 저장)
        OriginalUseControllerRotationYaw = OwnerCharacter->bUseControllerRotationYaw;
        OriginalOrientRotationToMovement = MovementComponent->bOrientRotationToMovement;
        OwnerCharacter->bUseControllerRotationYaw = false;
        if (MovementComponent)
        {
            MovementComponent->bOrientRotationToMovement = false;
        }
        // 활공 AnimMontage 재생
        if (GlideAnimMontage && OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
        {
            OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(GlideAnimMontage, 1.0f);
        }
    }
}

void URLGliderComponent::DeactivateGlider()
{
    if (bIsActive && MovementComponent && OwnerCharacter)
    {
        bIsActive = false;
        MovementComponent->GravityScale = OriginalGravityScale;
        MovementComponent->AirControl = OriginalAirControl;
        MovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed;
        if (GliderMesh) {
            GliderTargetScale = FVector(0,0,0); // 목표는 0 (줄어들며 사라짐)
            bGliderScaling = true;
        }
        SpawnPortalEffect();
        if (OwnerCharacter)
        {
            ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OwnerCharacter);
            if (Player)
            {
                Player->CameraTargetArmLength = 400.0f; // 원래 거리로 복귀
            }
        }
        // 글라이드 모드 해제: 진입 전 상태로 복구
        OwnerCharacter->bUseControllerRotationYaw = OriginalUseControllerRotationYaw;
        if (MovementComponent)
        {
            MovementComponent->bOrientRotationToMovement = OriginalOrientRotationToMovement;
        }
        // 활공 AnimMontage 중지
        if (GlideAnimMontage && OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
        {
            OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, GlideAnimMontage);
        }
    }
}

bool URLGliderComponent::IsGliderActive() const
{
    return bIsActive;
} 

void URLGliderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // 글라이드 모드일 때 하강 속도 제한 및 관성적 회전
    if (bIsActive && OwnerCharacter && MovementComponent)
    {
        FVector Vel = OwnerCharacter->GetVelocity();
        if (Vel.Z < -GliderMaxFallSpeed)
        {
            Vel.Z = -GliderMaxFallSpeed;
            MovementComponent->Velocity = Vel;
        }
        // 관성적 회전: 인풋이 있으면 그 방향, 없으면 마지막 방향/속도/전방
        FVector InputDir = OwnerCharacter->GetLastMovementInputVector();
        if (!InputDir.IsNearlyZero())
        {
            LastGlideInputDirection = InputDir.GetSafeNormal();
        }
        FVector TargetDir = LastGlideInputDirection;
        if (TargetDir.IsNearlyZero())
        {
            FVector Vel2D = OwnerCharacter->GetVelocity();
            Vel2D.Z = 0;
            if (!Vel2D.IsNearlyZero())
                TargetDir = Vel2D.GetSafeNormal();
            else
                TargetDir = OwnerCharacter->GetActorForwardVector();
        }
        FRotator CurrentRot = OwnerCharacter->GetActorRotation();
        FRotator TargetRot = TargetDir.Rotation();
        TargetRot.Pitch = 0.0f;
        TargetRot.Roll = 0.0f;
        float InterpSpeed = 0.5f;
        FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, InterpSpeed);
        OwnerCharacter->SetActorRotation(NewRot);
    }
    // 기존 GliderMesh 스케일 애니메이션
    if (bGliderScaling && GliderMesh)
    {
        FVector Current = GliderMesh->GetComponentScale();
        FVector NewScale = FMath::VInterpTo(Current, GliderTargetScale, DeltaTime, 6.0f);
        GliderMesh->SetWorldScale3D(NewScale);
        if (FVector::Dist(NewScale, GliderTargetScale) < 0.01f)
        {
            GliderMesh->SetWorldScale3D(GliderTargetScale);
            bGliderScaling = false;
            if (GliderTargetScale.IsNearlyZero())
            {
                GliderMesh->SetVisibility(false);
            }
        }
    }
    // 포탈 이펙트 스케일 애니메이션 (0→1→0)
    for (int32 i = ActivePortalEffects.Num() - 1; i >= 0; --i)
    {
        AActor* Portal = ActivePortalEffects[i];
        if (!Portal) { ActivePortalEffects.RemoveAt(i); continue; }
        FVector CurScale = Portal->GetActorScale3D();
        float Life = Portal->CustomTimeDilation; // 임시로 수명 저장용(0~2)
        if (Life < 1.0f)
        {
            // Scale 0→1로 확장
            float Alpha = FMath::Clamp(Life / 1.0f, 0.f, 1.f);
            Portal->SetActorScale3D(FVector(Alpha));
            Portal->CustomTimeDilation += DeltaTime;
        }
        else if (Life < 2.0f)
        {
            // Sclae 1→0으로 축소
            float Alpha = 1.f - FMath::Clamp((Life-1.0f) / 1.0f, 0.f, 1.f);
            Portal->SetActorScale3D(FVector(Alpha));
            Portal->CustomTimeDilation += DeltaTime;
        }
        else
        {
            Portal->Destroy();
            ActivePortalEffects.RemoveAt(i);
        }
    }
} 