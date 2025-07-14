// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/RLDragonAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

URLDragonAnimInstance::URLDragonAnimInstance()
{

}

void URLDragonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
	// null체크를 확실하게 해야함 애니메이션 블루프린트에서 크래시 방지를 위한 크래시 방지
	if (OwningPawn)
	{
		// 기본 애니메이션 속성 초기화 (URLAnimInstance에서 가져옴)
		Velocity = FVector::ZeroVector;
		GroundSpeed = 500.0f;
		bShouldMove = false;
		bIsFalling = false;
		ForwardSpeed = 0.0f;
		RightSpeed = 0.0f;

		// 드래곤 비행 상태 초기화
		bIsFlying = false;
		bIsFlyingAndMoving = false;
		FlightSpeed = 0.0f;
		FlightHeight = 0.0f;
		VerticalSpeed = 0.0f;

		// 비행 상태 임계값 설정
		FlightHeightThreshold = 230.0f;  // 250유닛 이상이면 비행으로 간주
		FlightSpeedThreshold = 50.0f;    // 50유닛/초 이상이면 움직이는 것으로 간주
		GroundHeightThreshold = 220.0f;   // 지면으로부터 180유닛 이하면 착지로 간주

		// Movement Component 초기화
		MovementComponent = nullptr;
		bWasFlyingLastFrame = false;
	}
}

void URLDragonAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	// 부모 클래스의 업데이트 먼저 호출
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
		return;
	}

	// 기본 애니메이션 로직 (URLAnimInstance에서 가져옴)
	// 캐릭터의 속도 구하기
	Velocity = OwningPawn->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	// 캐릭터 이동
	FVector Direction = FVector(Velocity.X, Velocity.Y, 0.0f);
	Direction.Normalize();
	FVector NewLocation = Direction * GroundSpeed * DeltaTimeX;

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
		
		// 속도 벡터를 캐릭터의 로컬 좌표계로 변환
		ForwardSpeed = FVector::DotProduct(Velocity, ForwardVector);
		RightSpeed = FVector::DotProduct(Velocity, RightVector);
	}

	// MovementComponent 참조 초기화 (한 번만)
	if (!MovementComponent && OwningPawn)
	{
		ACharacter* Character = Cast<ACharacter>(OwningPawn);
		if (Character)
		{
			MovementComponent = Character->GetCharacterMovement();
		}
	}

	// 드래곤 특화 비행 상태 업데이트
	UpdateFlightStates();

	// Movement Mode 업데이트
	UpdateMovementMode();
}

void URLDragonAnimInstance::UpdateFlightStates()
{
	if (!OwningPawn)
	{
		// Pawn이 없으면 모든 비행 상태를 false로 설정
		bIsFlying = false;
		bIsFlyingAndMoving = false;
		FlightSpeed = 0.0f;
		FlightHeight = 0.0f;
		VerticalSpeed = 0.0f;
		return;
	}

	// 비행 높이와 속도 계산
	CalculateFlightHeight();
	CalculateFlightSpeed();

	// 비행 상태 판단 (히스테리시스 로직 사용)
	// 현재 비행 중이 아닐 때: FlightHeightThreshold 이상이면 비행 시작
	// 현재 비행 중일 때: GroundHeightThreshold 이하면 착지
	if (!bIsFlying)
	{
		// 착지 상태에서 일정 높이 이상 올라가면 비행 시작
		if (FlightHeight > FlightHeightThreshold)
		{
			bIsFlying = true;
		}
	}
	else
	{
		// 비행 상태에서 지면 근처로 내려가면 착지
		if (FlightHeight <= GroundHeightThreshold)
		{
			bIsFlying = false;
		}
	}

	// 비행 중이면서 수평 이동 속도가 임계값 이상이면 "날면서 움직이는" 상태
	bIsFlyingAndMoving = bIsFlying && (FlightSpeed > FlightSpeedThreshold);
}

void URLDragonAnimInstance::CalculateFlightHeight()
{
	if (!OwningPawn || !OwningPawn->GetWorld())
	{
		FlightHeight = 0.0f;
		return;
	}

	// 드래곤의 현재 위치
	FVector DragonLocation = OwningPawn->GetActorLocation();
	
	// 지면까지의 거리를 Ray Cast로 측정
	FVector StartLocation = DragonLocation;
	FVector EndLocation = DragonLocation - FVector(0.0f, 0.0f, 5000.0f); // 아래쪽으로 5000유닛까지 체크

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningPawn);

	// 지면과의 충돌 체크
	bool bHit = OwningPawn->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_WorldStatic,
		QueryParams
	);

	// 디버그: 레이저(라인) 그리기
	// 충돌했다면 충돌 지점까지, 아니면 EndLocation까지 라인을 그립니다.
	DrawDebugLine(
		OwningPawn->GetWorld(),
		StartLocation,
		bHit ? HitResult.ImpactPoint : EndLocation,
		FColor::Red,
		false,          // 지속 시간 (0이면 한 프레임)
		0,              // depth priority
		2.0f            // 선 두께
	);
	if (bHit)
	{
		// 충돌 지점에 디버그 포인트 표시
		DrawDebugPoint(
			OwningPawn->GetWorld(),
			HitResult.ImpactPoint,
			10.0f,          // 크기
			FColor::Yellow,
			false,
			0
		);
	}

	if (bHit)
	{
		// 충돌한 지점과의 거리가 비행 높이
		FlightHeight = FVector::Dist(DragonLocation, HitResult.ImpactPoint);
	}
	else
	{
		// 충돌하지 않으면 매우 높은 곳에 있다고 가정
		FlightHeight = 5000.0f;
	}
}

void URLDragonAnimInstance::CalculateFlightSpeed()
{
	if (!OwningPawn)
	{
		FlightSpeed = 0.0f;
		VerticalSpeed = 0.0f;
		return;
	}

	// 현재 속도 벡터 가져오기
	FVector CurrentVelocity = OwningPawn->GetVelocity();
	
	// 수평 속도 계산 (XY 평면에서의 속도)
	FVector HorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
	FlightSpeed = HorizontalVelocity.Size();
	
	// 수직 속도 계산 (Z축 속도)
	VerticalSpeed = CurrentVelocity.Z;
}

void URLDragonAnimInstance::UpdateMovementMode()
{
	// MovementComponent가 없으면 리턴
	if (!MovementComponent)
	{
		return;
	}

	// 비행 상태가 변경되었는지 확인
	if (bIsFlying != bWasFlyingLastFrame)
	{
		if (bIsFlying)
		{
			// 비행 상태로 전환
			MovementComponent->SetMovementMode(MOVE_Flying);
			UE_LOG(LogTemp, Warning, TEXT("Dragon is now FLYING - Movement mode changed to Flying"));
		}
		else
		{
			// 착지 상태로 전환
			MovementComponent->SetMovementMode(MOVE_Walking);
			UE_LOG(LogTemp, Warning, TEXT("Dragon has LANDED - Movement mode changed to Walking"));
		}
	}

	// 매 프레임마다 이전 프레임 상태 업데이트 (상태 변경 여부와 관계없이)
	bWasFlyingLastFrame = bIsFlying;

}

