#include "RLGliderComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Character/RLCharacterPlayer.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

URLGliderComponent::URLGliderComponent()
{
    PrimaryComponentTick.bCanEverTick = true; // Tick 활성화
    bIsActive = false;
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // 글라이더 메시 컴포넌트 생성
    GliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GliderMesh"));
    GliderMesh->SetVisibility(false);
    GliderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GliderAsset(TEXT("/Game/Model/Wing/GliderFBX.GliderFBX"));
    if (GliderAsset.Succeeded())
    {
        GliderMesh->SetStaticMesh(GliderAsset.Object);
    }
    
    // 나이아가라 파티클 컴포넌트들 생성 (좌우 2개)
    GliderParticleLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GliderParticleLeft"));
    GliderParticleLeft->SetVisibility(false);
    GliderParticleLeft->SetAutoActivate(false);
    
    GliderParticleRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GliderParticleRight"));
    GliderParticleRight->SetVisibility(false);
    GliderParticleRight->SetAutoActivate(false);
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
        
        // 나이아가라 파티클들을 GliderMesh의 소켓들에 부착
        if (GliderParticleLeft && GliderMesh)
        {
            GliderParticleLeft->AttachToComponent(GliderMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftSocketName);
            
            // 나이아가라 시스템 설정
            if (GliderParticleSystem)
            {
                GliderParticleLeft->SetAsset(GliderParticleSystem);
            }
            
            UE_LOG(LogTemp, Log, TEXT("[GLIDER] Left particle attached to socket: %s"), *LeftSocketName.ToString());
        }
        
        if (GliderParticleRight && GliderMesh)
        {
            GliderParticleRight->AttachToComponent(GliderMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightSocketName);
            
            // 나이아가라 시스템 설정
            if (GliderParticleSystem)
            {
                GliderParticleRight->SetAsset(GliderParticleSystem);
            }
            
            UE_LOG(LogTemp, Log, TEXT("[GLIDER] Right particle attached to socket: %s"), *RightSocketName.ToString());
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
        
        // 나이아가라 파티클들 활성화
        if (GliderParticleLeft)
        {
            GliderParticleLeft->SetVisibility(true);
            GliderParticleLeft->Activate();
            UE_LOG(LogTemp, Log, TEXT("[GLIDER] Left particle activated"));
        }
        
        if (GliderParticleRight)
        {
            GliderParticleRight->SetVisibility(true);
            GliderParticleRight->Activate();
            UE_LOG(LogTemp, Log, TEXT("[GLIDER] Right particle activated"));
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
        
        // 나이아가라 파티클들 비활성화
        if (GliderParticleLeft)
        {
            GliderParticleLeft->Deactivate();
            GliderParticleLeft->SetVisibility(false);
            UE_LOG(LogTemp, Log, TEXT("[GLIDER] Left particle deactivated"));
        }
        
        if (GliderParticleRight)
        {
            GliderParticleRight->Deactivate();
            GliderParticleRight->SetVisibility(false);
            UE_LOG(LogTemp, Log, TEXT("[GLIDER] Right particle deactivated"));
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