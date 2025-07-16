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
	
	// 기본 콜리전 설정 (오버랩용)
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ArrowMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// 투사체 이동 컴포넌트 생성 (초기에는 비활성화)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = ArrowMesh;
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.3f;
	ProjectileMovement->SetActive(false); // 초기에는 비활성화 상태
	
	// 기본 설정
	Damage = 30;
	Speed = 2500.0f;
	LifeTime = 10.0f;
	ArrowDirectionOffset = FRotator(0.0f, 0.0f, 0.0f);
	bIsAttachedToSocket = false;
	AttachedMeshComponent = nullptr;
	AttachedSocketName = NAME_None;

	// 오버랩 이벤트 바인딩
	ArrowMesh->OnComponentBeginOverlap.AddDynamic(this, &ARLArrow::OnComponentBeginOverlap);
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
	
	// Direction이 FVector이므로 Rotation으로 변환
	FRotator TargetRotation = Direction.Rotation();
	SetActorRotation(TargetRotation);
	// + FRotator(0.0f, 5.0f, 5.0f)

	// 투사체 이동 설정 (발사 시에만 활성화)
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ArrowSpeed;
		ProjectileMovement->MaxSpeed = ArrowSpeed;
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * ArrowSpeed;
		ProjectileMovement->SetActive(true); // 발사 시에만 활성화
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
	
	// 투사체 이동은 InitializeArrow()에서 활성화됨
	// DetachFromSocket()에서는 활성화하지 않음
	
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
	
	// 투사체 이동 비활성화
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
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
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