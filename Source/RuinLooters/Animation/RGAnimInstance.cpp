// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RGAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

void URGAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
	// null인지 확인하지 않으면 애니메이션 블루프린트를 생성할 때 크러쉬 발생
	if (OwningPawn)
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 500.0f;
		bShouldMove = false;
		bIsFalling = false;
	}
}

void URGAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);

	OwningPawn = TryGetPawnOwner();
	// null인지 확인하지 않으면 애니메이션 블루프린트를 생성할 때 크러쉬 발생
	if (!OwningPawn)
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 500.0f;
		bShouldMove = false;
		bIsFalling = false;
		return;
	}

	// 캐릭터의 속도 구하기
	Velocity = OwningPawn->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	// 캐릭터 이동
	FVector Direction = FVector(Velocity.X, Velocity.Y, 0.0f);
	Direction.Normalize();
	FVector NewLocation =  Direction * GroundSpeed * DeltaTimeX;

	// 낙하 상태 판별
	UCharacterMovementComponent* MoveComp = Cast<UCharacterMovementComponent>(OwningPawn->GetMovementComponent());
	if (MoveComp)
	{
		CurrentAcceleration = MoveComp->GetCurrentAcceleration();
		bIsFalling = MoveComp->IsFalling();
	}
	else
	{
		CurrentAcceleration = FVector::ZeroVector;
	}

	if (GroundSpeed > 3 || CurrentAcceleration != FVector::ZeroVector)
	{
		bShouldMove = true;
	}
}
