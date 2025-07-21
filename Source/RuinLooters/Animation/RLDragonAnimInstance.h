// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "RLDragonAnimInstance.generated.h"

/**
 * 드래곤 전용 애니메이션 인스턴스 클래스
 * 비행 상태와 날면서 움직이는 상태를 관리합니다.
 * 비행 상태에 따라 CharacterMovementComponent의 이동 모드를 자동으로 변경합니다.
 */
UCLASS()
class RUINLOOTERS_API URLDragonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	URLDragonAnimInstance();

protected:
	// 기본 애니메이션 속성들 (URLAnimInstance에서 가져옴)
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

	// 드래곤 비행 상태 변수들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon Flight")
	bool bIsFlying;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon Flight")
	bool bIsFlyingAndMoving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon Flight")
	float FlightSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon Flight")
	float FlightHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dragon Flight")
	float VerticalSpeed;

	// 비행 상태 임계값들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Flight Settings")
	float FlightHeightThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Flight Settings")
	float FlightSpeedThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon Flight Settings")
	float GroundHeightThreshold;

	// CharacterMovementComponent 참조
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;

	// 이전 비행 상태 (상태 변경 감지용)
	bool bWasFlyingLastFrame;

	// OwningPawn 참조 (URLAnimInstance에서 가져옴)
	UPROPERTY();
	APawn* OwningPawn = nullptr;

public:
	// URLAnimInstance 기능들
	FORCEINLINE void SetShouldMove(bool ShouldMove) { bShouldMove = ShouldMove; };

	virtual void NativeInitializeAnimation() override;

	// 애니메이션 업데이트 함수 오버라이드
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
	// 드래곤 비행 상태 업데이트 함수
	void UpdateFlightStates();

	// 비행 높이 계산 함수
	void CalculateFlightHeight();

	// 비행 속도 계산 함수
	void CalculateFlightSpeed();

	// Movement Mode 업데이트 함수
	void UpdateMovementMode();
};
