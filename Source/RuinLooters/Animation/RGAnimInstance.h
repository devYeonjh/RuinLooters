// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RGAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
    // 애님 블루프린트에서 사용 가능한 속성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float GroundSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector Velocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    uint8 bShouldMove : 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    uint8 bIsFalling : 1;
    UPROPERTY()
    FVector CurrentAcceleration;

public:
    FORCEINLINE void SetShouldMove(bool ShouldMove) { bShouldMove = ShouldMove; };

protected:
    // 애님 초기화 시점에 호출
    virtual void NativeInitializeAnimation() override;

    // 매 프레임 델타타임만큼 호출
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
	
    APawn* OwningPawn = nullptr;
};
