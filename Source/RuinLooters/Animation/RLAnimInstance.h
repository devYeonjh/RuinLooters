// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RLAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
    // 애니메이션 블루프린트에서 사용하는 속성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float GroundSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector Velocity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bShouldMove;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsFalling;
    UPROPERTY()
    FVector CurrentAcceleration;

    // 블렌드 스페이스용 방향 속도 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float ForwardSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float RightSpeed;

    // 입력값 기반 블렌드 스페이스용 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float InputForwardSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float InputRightSpeed;

    // 폼 체인지 (검/활 모드)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsSword;

    // 에이밍 상태 (활 모드 전용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsAiming;

public:
    FORCEINLINE void SetShouldMove(bool ShouldMove) { bShouldMove = ShouldMove; };
    
    // 입력값 설정 함수들
    FORCEINLINE void SetInputForwardSpeed(float NewInputForwardSpeed) { InputForwardSpeed = 500 * NewInputForwardSpeed; };
    FORCEINLINE void SetInputRightSpeed(float NewInputRightSpeed) { InputRightSpeed = 500 * NewInputRightSpeed; };

protected:
    // 애니메이션 초기화 시점에 호출
    virtual void NativeInitializeAnimation() override;

    // 매 프레임 델타타임만큼 호출
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
	
    APawn* OwningPawn = nullptr;
};



