// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/RLCharacterEnemyDragon.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Character/RLCharacterPlayer.h"
#include "Character/RLCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Pool/RLProjectilePool.h"
#include "../Projectile/RLProjectile.h"
#include "../Projectile/RLDragonProjectile.h"
#include "AIController.h"
#include "Engine/DamageEvents.h"
#include "Particles/ParticleSystem.h"

ARLCharacterEnemyDragon::ARLCharacterEnemyDragon()
{
	// 기본 스탯 설정
	    CurrentHp = 2000.0f;
    MaxHp = 2000.0f;
	AttackDamage = 50;
	Range = 300.0f;
	AttackSpeed = 1.5f;
	Defence = 20;
	
	// 공격 가능 상태로 시작
	bIsCanAttack = true;
	
	// 캡슐 콜리전 공격 설정
	CapsuleAttackRadius = 200.0f;
	CapsuleAttackHeight = 400.0f;
	
	// 브레스 공격 설정
	BreathDamage = 75;
	BreathRange = 1000.0f;
	
	// 투사체 풀 초기화
	ProjectilePool = nullptr;
	
	// 브레스 투사체 콜리전 설정 초기화
	BreathProjectileCollisionRadius = 30.0f;  // 드래곤 브레스 투사체 반지름
	BreathProjectileCollisionHeight = 60.0f;  // 드래곤 브레스 투사체 높이
	
	// 폭발 파티클 템플릿 초기화
	ExplosionParticleTemplate = nullptr;
	
	// 애니메이션 몽타주 초기화
	AttackMontage = nullptr;
	DieMontage = nullptr;
	GroundBreathMontage = nullptr;
	SkyBreathMontage = nullptr;
	
	// 사운드 초기화
	AttackSound = nullptr;
	HitSound = nullptr;
	DieSound = nullptr;
	
	// 기본 캐릭터 설정
	PrimaryActorTick.bCanEverTick = true;
	
	// 캡슐 콜리전 설정
	GetCapsuleComponent()->SetCapsuleSize(100.0f, 200.0f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ARLCharacterEnemyDragon::BeginPlay()
{
	Super::BeginPlay();
	
	// 애니메이션 인스턴스 초기화
	AnimInstance = GetMesh()->GetAnimInstance();
	
	// 몽타주 종료 콜백 바인딩
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ARLCharacterEnemyDragon::OnMontageEnded);
	}
	
	// 투사체 풀 초기화
	if (ProjectileClass)
	{
		ProjectilePool = NewObject<URLProjectilePool>(this);
		ProjectilePool->InitializePool(GetWorld(), ProjectileClass, 10);
		UE_LOG(LogTemp, Log, TEXT("Dragon projectile pool initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dragon ProjectileClass is not set"));
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void ARLCharacterEnemyDragon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 투사체 풀 정리
	if (ProjectilePool)
	{
		ProjectilePool->CleanupActiveProjectiles();
		UE_LOG(LogTemp, Warning, TEXT("Dragon EndPlay: Cleaned up projectile pool"));
	}
	
	Super::EndPlay(EndPlayReason);
}

float ARLCharacterEnemyDragon::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 이미 죽었다면 데미지 무시
	if (CurrentHp <= 0)
	{
		return 0.0f;
	}
	
	// 방어력 적용
	float ActualDamage = FMath::Max(1.0f, DamageAmount - Defence);
	CurrentHp = FMath::Max(0.0f, CurrentHp - ActualDamage);
	
	// 타격 파티클 이펙트 생성
	if (HitParticleTemplate)
	{
		FVector HitLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f); // 드래곤 중앙 위치 (높게)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticleTemplate, HitLocation);
	}
	
	// 피격 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	
	// 데미지 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Dragon received %.1f damage (actual: %.1f), HP: %.1f/%.1f"), DamageAmount, ActualDamage, CurrentHp, MaxHp);
	
	// HP가 0 이하가 되면 죽음 처리
	if (CurrentHp <= 0)
	{
		Die();
	}
	
	return ActualDamage;
}

void ARLCharacterEnemyDragon::Heal(int32 HealAmount)
{
	if (CurrentHp <= 0)
	{
		return;
	}
	
	float OldHp = CurrentHp;
	CurrentHp = FMath::Min(MaxHp, CurrentHp + HealAmount);
	
	UE_LOG(LogTemp, Warning, TEXT("Dragon healed %.1f HP: %.1f/%.1f"), CurrentHp - OldHp, CurrentHp, MaxHp);
}

void ARLCharacterEnemyDragon::Die()
{
	if (CurrentHp > 0)
	{
		CurrentHp = 0;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Dragon has died"));
	
	// 죽음 사운드 재생
	if (DieSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
	}
	
	// 죽음 애니메이션 재생
	if (DieMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(DieMontage);
	}
	
	// 콜리전 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 더 이상 공격할 수 없음
	bIsCanAttack = false;
	
	// 죽음 델리게이트 호출
	if (DragonDie.IsBound())
	{
		DragonDie.Broadcast();
	}
	
	// 3초 후 액터 파괴
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		Destroy();
	}, 3.0f, false);
}

void ARLCharacterEnemyDragon::Attack()
{
	// 공격 가능 상태 확인
	if (!bIsCanAttack || CurrentHp <= 0)
	{
		return;
	}
	
	// 공격 사운드 재생
	PlayAttackSound();

	// 공격 애니메이션 재생
	if (AttackMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(AttackMontage);
	}
	
	// 공격 로그
	UE_LOG(LogTemp, Warning, TEXT("Dragon is attacking with damage: %d"), AttackDamage);
	
	// 공격 중 움직임 비활성화
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	
	// 공격 쿨다운 시작 (애니메이션 종료 시 OnMontageEnded에서 재설정)
	bIsCanAttack = false;
}

void ARLCharacterEnemyDragon::CallAttackCollision()
{
	// 드래곤의 현재 위치와 방향
	FVector StartLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	FVector RightVector = GetActorRightVector();

	// 공격 범위 계산 (앞쪽으로 Range만큼)
	FVector EndLocation = StartLocation + (ForwardVector * Range);

	// 드래곤의 현재 회전을 쿼터니언으로 가져오기
	FQuat DragonQuat = GetActorRotation().Quaternion();

	// 드래곤의 로컬 Up 축을 기준으로 90도 회전 (드래곤 기준 상대적 회전)
	FQuat ExtraRot = FQuat(RightVector, FMath::DegreesToRadians(90.f));

	// 최종 회전값 생성 (드래곤 기준 상대적)
	FQuat CapsuleRot = ExtraRot * DragonQuat;

	// 캡슐 트레이스 파라미터 설정
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	// 캡슐 트레이스 실행
	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		CapsuleRot,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeCapsule(CapsuleAttackRadius, CapsuleAttackHeight),
		QueryParams
	);


	FColor DebugColor = bHit ? FColor::Red : FColor::Green;
	DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) / 2, CapsuleAttackHeight, CapsuleAttackRadius,
		CapsuleRot, DebugColor, false, 2.0f);


	// 피격된 액터들 처리
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				// 플레이어인지 확인
				ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(HitActor);
				if (Player)
				{
									// 플레이어에게 데미지 적용
				FDamageEvent DamageEvent;
				Player->TakeDamage((float)AttackDamage, DamageEvent, nullptr, this);
				UE_LOG(LogTemp, Warning, TEXT("Dragon hit player for %d damage"), AttackDamage);
				}
			}
		}
	}
}

void ARLCharacterEnemyDragon::PlayAttackSound()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
}



void ARLCharacterEnemyDragon::BreathAttack()
{
	// 브레스 공격 가능 상태 확인
	if (!bIsCanAttack || CurrentHp <= 0)
	{
		return;
	}
	
	// 브레스 공격 사운드 재생
	PlayAttackSound();
	
	// 현재 MovementMode에 따라 다른 브레스 애니메이션 재생
	EMovementMode CurrentMovementMode = GetCharacterMovement()->MovementMode;
	
	if (CurrentMovementMode == MOVE_Walking)
	{
		AnimInstance->Montage_Play(GroundBreathMontage);
		UE_LOG(LogTemp, Log, TEXT("Dragon using ground breath montage"));
	}
	else if (CurrentMovementMode == MOVE_Flying)
	{
		AnimInstance->Montage_Play(SkyBreathMontage);
		UE_LOG(LogTemp, Log, TEXT("Dragon using sky breath montage"));
	}
	
	// 브레스 공격 로그
	UE_LOG(LogTemp, Warning, TEXT("Dragon is breathing fire with damage: %d"), BreathDamage);
	
	// 브레스 공격 중 움직임 비활성화
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	
	// 공격 쿨다운 시작
	bIsCanAttack = false;
}

void ARLCharacterEnemyDragon::FireBreathProjectile()
{
	if (!ProjectilePool)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dragon projectile pool is not initialized"));
		return;
	}
	
	// 투사체 풀에서 투사체 가져오기 (ARLDragonProjectile로 캐스트)
	ARLDragonProjectile* DragonProjectile = Cast<ARLDragonProjectile>(ProjectilePool->GetProjectile());
	if (!DragonProjectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get dragon projectile from pool"));
		return;
	}
	
	// 발사 위치 설정
	FVector DragonMouthLocation = GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0.0f, 0.0f, 50.0f);
	
	// 타겟 방향 계산 (AI 컨트롤러의 포커스 타겟 사용)
	FVector FireDirection = GetActorForwardVector(); // 기본값
	
	// AI 컨트롤러에서 현재 포커스 타겟 가져오기
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (AActor* FocusActor = AIController->GetFocusActor())
		{
			// 타겟의 위치로 방향 계산
			FVector TargetLocation = FocusActor->GetActorLocation();
			FireDirection = (TargetLocation - DragonMouthLocation).GetSafeNormal();
			
			UE_LOG(LogTemp, Log, TEXT("Dragon firing at focus target: %s"), *TargetLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Dragon has no focus target, firing forward"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dragon has no AI controller, firing forward"));
	}
	
	// 투사체 가시성 및 콜리전 활성화 (풀에서 가져온 경우 숨겨져 있을 수 있음)
	DragonProjectile->SetActorHiddenInGame(false);
	DragonProjectile->SetActorEnableCollision(true);
	
	// 드래곤 설정값으로 투사체 설정 (캐릭터에서 설정한 콜리전 값 사용)
	DragonProjectile->SetupWithDragonSettings(
		BreathProjectileCollisionRadius,  // 캐릭터에서 설정한 반지름
		BreathProjectileCollisionHeight,  // 캐릭터에서 설정한 높이
		BreathDamage,                     // 캐릭터에서 설정한 데미지
		1500.0f                           // 브레스 속도
	);
	
	// 폭발 파티클 템플릿 설정 (발사체 파티클과 별개)
	if (ExplosionParticleTemplate)
	{
		DragonProjectile->SetExplosionParticleTemplate(ExplosionParticleTemplate);
	}
	

	
	// 투사체 초기화 및 발사 (자체 생존 시간으로 자동 반환)
	DragonProjectile->InitializeProjectile(DragonMouthLocation, FireDirection, DragonProjectile->GetProjectileSettings());
	
	// 투사체 소유자 설정
	DragonProjectile->SetOwner(this);
	
	UE_LOG(LogTemp, Warning, TEXT("Dragon breath projectile fired - Location: %s, Direction: %s, Radius: %f, Height: %f"), 
           *DragonMouthLocation.ToString(), *FireDirection.ToString(), BreathProjectileCollisionRadius, BreathProjectileCollisionHeight);
	
	// 타이머 제거 - 대신 발사체 자체의 생존 시간(LifeTime)을 이용하여 자동 반환
	// ARLProjectile::OnLifeTimeExpired()에서 자동으로 풀에 반환됨
	
	UE_LOG(LogTemp, Log, TEXT("Dragon breath projectile fired with auto-return on lifetime expiration"));
}

void ARLCharacterEnemyDragon::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 공격 몽타주가 끝나면 다시 공격 가능 상태로 설정
	if (AttackMontage == Montage || GroundBreathMontage == Montage || SkyBreathMontage == Montage)
	{
		bIsCanAttack = true;
		
		// 움직임 다시 활성화 (원래 상태로 복원)
		if (GroundBreathMontage == Montage)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			UE_LOG(LogTemp, Warning, TEXT("Dragon ground breath montage ended, returning to walking"));
		}
		else if (SkyBreathMontage == Montage)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			UE_LOG(LogTemp, Warning, TEXT("Dragon sky breath montage ended, returning to flying"));
		}
		else
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			UE_LOG(LogTemp, Warning, TEXT("Dragon attack montage ended, can attack and move again"));
		}
	}
}

