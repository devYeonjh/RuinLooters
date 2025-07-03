// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RLAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"

void URLAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
	// null���� Ȯ������ ������ �ִϸ��̼� ��������Ʈ�� ������ �� ũ���� �߻�
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
	// null���� Ȯ������ ������ �ִϸ��̼� ��������Ʈ�� ������ �� ũ���� �߻�
	if (!OwningPawn)
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 500.0f;
		bShouldMove = false;
		bIsFalling = false;
		return;
	}

	// ĳ������ �ӵ� ���ϱ�
	Velocity = OwningPawn->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	// ĳ���� �̵�
	FVector Direction = FVector(Velocity.X, Velocity.Y, 0.0f);
	Direction.Normalize();
	FVector NewLocation =  Direction * GroundSpeed * DeltaTimeX;

	// ���� ���� �Ǻ�
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



