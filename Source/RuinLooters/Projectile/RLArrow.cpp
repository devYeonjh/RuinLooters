// Fill out your copyright notice in the Description page of Project Settings.

#include "RLArrow.h"
#include "Character/RLCharacterEnemy.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

ARLArrow::ARLArrow()
{
	PrimaryActorTick.bCanEverTick = false;

	// 스태틱 메시 컴포넌트 생성
	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	RootComponent = ArrowMesh;
	
	// 기본 콜리전 설정
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ArrowMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ArrowMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	
	// 투사체 이동 컴포넌트 생성
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = ArrowMesh;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f;

	// 기본 설정
	Damage = 30;
	Speed = 2500.0f;
	LifeTime = 10.0f;
	bIsAttachedToSocket = false;
	AttachedMeshComponent = nullptr;
	AttachedSocketName = NAME_None;

	// 충돌 이벤트 바인딩
	ArrowMesh->OnComponentHit.AddDynamic(this, &ARLArrow::OnHit);
}

void ARLArrow::BeginPlay()
{
	Super::BeginPlay();
}

void ARLArrow::InitializeArrow(FVector StartLocation, FVector Direction, int32 ArrowDamage, float ArrowSpeed)
{
	// 설정 적용
	Damage = ArrowDamage;
	Speed = ArrowSpeed;
	
	// 위치와 회전 설정
	SetActorLocation(StartLocation);
	SetActorRotation(Direction.Rotation());
	
	// 투사체 이동 설정
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ArrowSpeed;
		ProjectileMovement->MaxSpeed = ArrowSpeed;
		ProjectileMovement->Velocity = Direction * ArrowSpeed;
		ProjectileMovement->SetActive(true);
	}
	
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
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
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
	
	// 투사체 이동 활성화
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(true);
	}
	
	// 콜리전 활성화
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
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
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}
	
	// 타이머 정리
	GetWorldTimerManager().ClearTimer(LifeTimeHandle);
	
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
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ARLArrow::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	// 소켓에 부착된 상태에서는 충돌 무시
	if (bIsAttachedToSocket)
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
			DamageEvent.HitInfo = Hit;
			
			OtherActor->TakeDamage(Damage, DamageEvent, nullptr, this);
			UE_LOG(LogTemp, Log, TEXT("Arrow hit enemy for %d damage"), Damage);
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