#include "RLGliderComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    }
}

bool URLGliderComponent::IsGliderActive() const
{
    return bIsActive;
} 

void URLGliderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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