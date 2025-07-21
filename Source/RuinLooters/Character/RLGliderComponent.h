#pragma once
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPtr.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "RLGliderComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLGliderComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    URLGliderComponent();

    void ActivateGlider();
    void DeactivateGlider();
    bool IsGliderActive() const;
    void SpawnPortalEffect();
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool bIsActive;
    float OriginalGravityScale;
    float OriginalAirControl;
    float OriginalMaxWalkSpeed;
    float GliderMaxFallSpeed = 600.0f;    // 글라이드 모드용 낙하속도(절반)
    UPROPERTY();
    class ACharacter* OwnerCharacter;
    UPROPERTY();
    class UCharacterMovementComponent* MovementComponent;
    UPROPERTY(VisibleAnywhere, Category="Glider")
    UStaticMeshComponent* GliderMesh;
    
    // 나이아가라 파티클 컴포넌트들 (좌우 2개)
    UPROPERTY(VisibleAnywhere, Category="Glider")
    UNiagaraComponent* GliderParticleLeft;
    
    UPROPERTY(VisibleAnywhere, Category="Glider")
    UNiagaraComponent* GliderParticleRight;
    
    // 나이아가라 시스템 에셋
    UPROPERTY(EditDefaultsOnly, Category="Glider")
    UNiagaraSystem* GliderParticleSystem;
    
    // 파티클이 부착될 소켓 이름들
    UPROPERTY(EditDefaultsOnly, Category="Glider")
    FName LeftSocketName = TEXT("Left");
    
    UPROPERTY(EditDefaultsOnly, Category="Glider")
    FName RightSocketName = TEXT("Right");
    
    UPROPERTY(EditDefaultsOnly, Category="Glider")
    float GliderGravityScale = 0.2f;
    UPROPERTY(EditDefaultsOnly, Category="Glider")
    float GliderAirControl = 0.7f;
    UPROPERTY(EditDefaultsOnly, Category="Glider")
    float GliderMaxWalkSpeed = 400.0f;
    FVector GliderTargetScale = FVector(1,1,1);
    bool bGliderScaling = false;
    UPROPERTY(EditDefaultsOnly, Category="Glider|Effect")
    TSoftClassPtr<AActor> PortalEffectClass;
    TArray<AActor*> ActivePortalEffects;
    FVector LastGlideInputDirection = FVector::ZeroVector; // 글라이드 관성 회전용
    bool OriginalUseControllerRotationYaw = false;
    bool OriginalOrientRotationToMovement = true;
    UPROPERTY(EditDefaultsOnly, Category="Glider|Animation")
    UAnimMontage* GlideAnimMontage = nullptr;
}; 