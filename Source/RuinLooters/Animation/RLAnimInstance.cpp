// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RLAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

void URLAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
	// null체크를 확실하게 해야함 애니메이션 블루프린트에서 크래시 방지를 위한 크래시 방지
	if (OwningPawn)
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 500.0f;
		bShouldMove = false;
		bIsFalling = false;
	}
}

void URLAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);

	OwningPawn = TryGetPawnOwner();
	// null체크를 확실하게 해야함 애니메이션 블루프린트에서 크래시 방지를 위한 크래시 방지
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
	else
	{
		bShouldMove = false;
	}
}



