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
    // �ִ� ��������Ʈ���� ��� ������ �Ӽ�
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
    // �ִ� �ʱ�ȭ ������ ȣ��
    virtual void NativeInitializeAnimation() override;

    // �� ������ ��ŸŸ�Ӹ�ŭ ȣ��
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
	
    APawn* OwningPawn = nullptr;
};
