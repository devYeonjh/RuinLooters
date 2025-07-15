// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RLAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/RLCharacterBase.h"
#include "Character/RLCharacterPlayer.h"

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
		ForwardSpeed = 0.0f;
		RightSpeed = 0.0f;
		InputForwardSpeed = 0.0f;
		InputRightSpeed = 0.0f;
		IsSword = true;
		bIsAiming = false;
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
		ForwardSpeed = 0.0f;
		RightSpeed = 0.0f;
		InputForwardSpeed = 0.0f;
		InputRightSpeed = 0.0f;
		IsSword = true;
		bIsAiming = false;
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

	// 블렌드 스페이스용 방향 속도 계산
	if (OwningPawn)
	{
		FVector ForwardVector = OwningPawn->GetActorForwardVector();
		FVector RightVector = OwningPawn->GetActorRightVector();
		
		// 속도 벡터를 캐릭터의 로컬 좌표계로 변환 (실제 속도 기반)
		ForwardSpeed = FVector::DotProduct(Velocity, ForwardVector);
		RightSpeed = FVector::DotProduct(Velocity, RightVector);
		
		// 입력값 기반 블렌드 스페이스용 속도는 이미 Move 함수에서 설정됨
		// InputForwardSpeed와 InputRightSpeed는 SetInputForwardSpeed/SetInputRightSpeed 함수로 설정
		
		// 플레이어 캐릭터인 경우 IsSword 값 업데이트
		if (ARLCharacterPlayer* PlayerCharacter = Cast<ARLCharacterPlayer>(OwningPawn))
		{
			IsSword = PlayerCharacter->GetbIsSword();
			bIsAiming = PlayerCharacter->GetbIsAiming();
		}
	}
}



