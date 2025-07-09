#pragma once
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPtr.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
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
    class ACharacter* OwnerCharacter;
    class UCharacterMovementComponent* MovementComponent;
    UPROPERTY(VisibleAnywhere, Category="Glider")
    UStaticMeshComponent* GliderMesh;
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