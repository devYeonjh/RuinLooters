// Fill out your copyright notice in the Description page of Project Settings.

#include "RLArrow.h"
#include "Character/RLCharacterEnemy.h"
#include "Character/RLCharacterPlayer.h"
#include "Character/RLCharacterEnemyDragon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"

ARLArrow::ARLArrow()
{
	PrimaryActorTick.bCanEverTick = false;

	// 스피어 콜리전 컴포넌트 생성 (루트 컴포넌트)
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	
	// 스피어 콜리전 설정 (오버랩용)
	SphereCollision->SetSphereRadius(60.0f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// 스태틱 메시 컴포넌트 생성
	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(SphereCollision);
	
	// 스태틱 메시 콜리전 설정 (비주얼용)
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// 투사체 이동 컴포넌트 생성 (초기에는 비활성화)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f;
	ProjectileMovement->SetActive(false); // 초기에는 비활성화 상태
	
	// 기본 설정
	Damage = 50;
	Speed = 2500.0f;
	LifeTime = 10.0f;
	ArrowDirectionOffset = FRotator(0.0f, 0.0f, 0.0f);
	bIsAttachedToSocket = false;
	AttachedMeshComponent = nullptr;
	AttachedSocketName = NAME_None;
	
	// 파티클 설정 초기화
	TrailParticleTemplate = nullptr;
	HitParticleTemplate = nullptr;
	TrailParticleComponent = nullptr;
	TrailParticleOffset = FVector(0.0f, 0.0f, 0.0f);
	HitParticleOffset = FVector(0.0f, 0.0f, 0.0f);
}

void ARLArrow::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩 (스피어 콜리전에 바인딩)
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ARLArrow::OnComponentBeginOverlap);
}

void ARLArrow::InitializeArrow(FVector StartLocation, APlayerController* PlayerController, int32 ArrowDamage, float ArrowSpeed)
{
	// 설정 적용
	Damage = ArrowDamage;
	Speed = ArrowSpeed;
	
	// 카메라의 Forward Vector로 목표 위치 계산
	FVector TargetLocation;
	FVector Direction;
	
	if (PlayerController && PlayerController->GetPawn())
	{
		// 카메라 컴포넌트 가져오기
		UCameraComponent* CameraComponent = PlayerController->GetPawn()->FindComponentByClass<UCameraComponent>();
		if (CameraComponent)
		{
			FVector CameraLocation = CameraComponent->GetComponentLocation();
			FVector CameraForward = CameraComponent->GetForwardVector();
			
			// 카메라 앞쪽으로 10000 거리만큼 떨어진 위치를 목표로 설정
			TargetLocation = CameraLocation + (CameraForward * 10000.0f);
			Direction = (TargetLocation - StartLocation).GetSafeNormal();
		}
		else
		{
			// 카메라 컴포넌트가 없으면 플레이어의 Forward Vector 사용
			FVector PlayerForward = PlayerController->GetPawn()->GetActorForwardVector();
			TargetLocation = StartLocation + (PlayerForward * 10000.0f);
			Direction = PlayerForward;
		}
	}
	else
	{
		// PlayerController가 없으면 앞쪽으로 발사
		Direction = FVector::ForwardVector;
		TargetLocation = StartLocation + (Direction * 10000.0f);
	}
	
	// 위치와 회전 설정
	SetActorLocation(StartLocation);
	FRotator TargetRotation = Direction.Rotation();
	SetActorRotation(TargetRotation);

	// 투사체 이동 설정 (발사 시에만 활성화)
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ArrowSpeed;
		ProjectileMovement->MaxSpeed = ArrowSpeed;
		ProjectileMovement->Velocity = Direction * ArrowSpeed;
		ProjectileMovement->SetActive(true); // 발사 시에만 활성화
	}
	
	// 트레일 파티클 생성
	CreateTrailParticle();
	
	// 라이프타임 타이머 시작
	GetWorldTimerManager().SetTimer(LifeTimeHandle, this, &ARLArrow::OnLifeTimeExpired, LifeTime, false);
	
	UE_LOG(LogTemp, Log, TEXT("Arrow initialized with damage: %d, speed: %f"), Damage, Speed);
}

void ARLArrow::AttachToSocket(USkeletalMeshComponent* TargetMesh, FName SocketName)
{
	if (!TargetMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARLArrow::AttachToSocket - TargetMesh is null"));
		return;
	}
	
	// 투사체 이동 비활성화 (소켓에 부착 시)
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}
	
	// 소켓에 부착
	AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	
	// 상태 저장
	bIsAttachedToSocket = true;
	AttachedMeshComponent = TargetMesh;
	AttachedSocketName = SocketName;
	
	// 콜리전 비활성화
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	UE_LOG(LogTemp, Log, TEXT("Arrow attached to socket: %s"), *SocketName.ToString());
}

void ARLArrow::DetachFromSocket()
{
	if (!bIsAttachedToSocket)
	{
		return;
	}
	
	// 소켓에서 분리
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	// 투사체 이동은 InitializeArrow()에서 활성화됨
	// DetachFromSocket()에서는 활성화하지 않음
	
	// 콜리전 활성화
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// 상태 초기화
	bIsAttachedToSocket = false;
	AttachedMeshComponent = nullptr;
	AttachedSocketName = NAME_None;
	
	UE_LOG(LogTemp, Log, TEXT("Arrow detached from socket"));
}

void ARLArrow::DeactivateArrow()
{
	// 화살 비활성화 (풀링용)
	SetActorHiddenInGame(true);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 투사체 이동 비활성화
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}
	
	// 타이머 정리
	GetWorldTimerManager().ClearTimer(LifeTimeHandle);
	
	// 트레일 파티클 제거
	if (TrailParticleComponent)
	{
		TrailParticleComponent->DestroyComponent();
		TrailParticleComponent = nullptr;
	}
	
	// 소켓 분리
	if (bIsAttachedToSocket)
	{
		DetachFromSocket();
	}
}

void ARLArrow::ActivateArrow()
{
	// 화살 활성화 (풀링용)
	SetActorHiddenInGame(false);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// ProjectileMovement는 InitializeArrow()에서 발사할 때만 활성화
	// 여기서는 활성화하지 않음
}

void ARLArrow::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 소켓에 부착된 상태에서는 오버랩 무시
	if (bIsAttachedToSocket)
	{
		return;
	}

	ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OtherActor);

	if (Player)
	{
		return;
	}
	
	// 자기 자신이나 소유자는 무시
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// 적에게 데미지 적용
		if (ARLCharacterEnemy* Enemy = Cast<ARLCharacterEnemy>(OtherActor))
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.Damage = Damage;
			DamageEvent.HitInfo = SweepResult;
			
			OtherActor->TakeDamage(Damage, DamageEvent, nullptr, this);
			UE_LOG(LogTemp, Log, TEXT("Arrow overlapped with enemy for %d damage"), Damage);
			
			// 히트 파티클 생성
			CreateHitParticle(SweepResult.Location);
		}

		// 적에게 데미지 적용
		if (ARLCharacterEnemyDragon* Enemy = Cast<ARLCharacterEnemyDragon>(OtherActor))
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.Damage = Damage;
			DamageEvent.HitInfo = SweepResult;

			OtherActor->TakeDamage(Damage, DamageEvent, nullptr, this);
			UE_LOG(LogTemp, Log, TEXT("Arrow overlapped with enemy for %d damage"), Damage);

			// 히트 파티클 생성
			CreateHitParticle(SweepResult.Location);
		}
		
		// 화살 비활성화
		DeactivateArrow();
	}
}

void ARLArrow::OnLifeTimeExpired()
{
	// 라이프타임 만료 시 화살 비활성화
	DeactivateArrow();
	UE_LOG(LogTemp, Log, TEXT("Arrow lifetime expired"));
}

void ARLArrow::CreateTrailParticle()
{
	if (TrailParticleTemplate)
	{
		// 기존 트레일 파티클이 있다면 제거
		if (TrailParticleComponent)
		{
			TrailParticleComponent->DestroyComponent();
		}
		
		// 트레일 파티클 컴포넌트 생성
		TrailParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			TrailParticleTemplate,
			ArrowMesh,
			NAME_None,
			TrailParticleOffset,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
		
		UE_LOG(LogTemp, Log, TEXT("Arrow trail particle created"));
	}
}

void ARLArrow::CreateHitParticle(FVector HitLocation)
{
	if (HitParticleTemplate)
	{
		// 히트 파티클 생성 (월드에 직접 스폰)
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticleTemplate,
			HitLocation + HitParticleOffset,
			FRotator::ZeroRotator,
			true
		);
		
		UE_LOG(LogTemp, Log, TEXT("Arrow hit particle created at location: %s"), *HitLocation.ToString());
	}
} 