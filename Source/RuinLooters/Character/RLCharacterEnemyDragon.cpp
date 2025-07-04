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

ARLCharacterEnemyDragon::ARLCharacterEnemyDragon()
{
	// 기본 스탯 설정
	CurrentHp = 2000;
	MaxHp = 2000;
	AttackDamage = 50;
	Range = 400.0f;
	AttackSpeed = 1.5f;
	Defence = 20;
	
	// 공격 가능 상태로 시작
	bIsCanAttack = true;
	
	// 캡슐 콜리전 공격 설정
	CapsuleAttackRadius = 200.0f;
	CapsuleAttackHeight = 100.0f;
	
	// 애니메이션 몽타주 초기화
	CurrentMontage = nullptr;
	DieMontage = nullptr;
	
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
}

void ARLCharacterEnemyDragon::TakeDragonDamage(int32 ReceivedDamage)
{
	// 이미 죽었다면 데미지 무시
	if (CurrentHp <= 0)
	{
		return;
	}
	
	// 방어력 적용
	int32 ActualDamage = FMath::Max(1, ReceivedDamage - Defence);
	CurrentHp = FMath::Max(0, CurrentHp - ActualDamage);
	
	// 피격 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	
	// 데미지 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Dragon received %d damage (actual: %d), HP: %d/%d"), ReceivedDamage, ActualDamage, CurrentHp, MaxHp);
	
	// HP가 0 이하가 되면 죽음 처리
	if (CurrentHp <= 0)
	{
		Die();
	}
}

void ARLCharacterEnemyDragon::Heal(int32 HealAmount)
{
	if (CurrentHp <= 0)
	{
		return;
	}
	
	int32 OldHp = CurrentHp;
	CurrentHp = FMath::Min(MaxHp, CurrentHp + HealAmount);
	
	UE_LOG(LogTemp, Warning, TEXT("Dragon healed %d HP: %d/%d"), CurrentHp - OldHp, CurrentHp, MaxHp);
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
		CurrentMontage = DieMontage;
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
	
	// 공격 로그
	UE_LOG(LogTemp, Warning, TEXT("Dragon is attacking with damage: %d"), AttackDamage);
	
	// 캡슐 콜리전 공격 트레이스 실행
	CapsuleAttackTrace();
	
	// 공격 쿨다운 시작
	bIsCanAttack = false;
	
	// 공격 속도에 따른 쿨다운 타이머 설정
	FTimerHandle AttackCooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(AttackCooldownTimer, [this]()
	{
		bIsCanAttack = true;
	}, AttackSpeed, false);
}

void ARLCharacterEnemyDragon::PlayAttackSound()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
}

void ARLCharacterEnemyDragon::CapsuleAttackTrace()
{
	// 드래곤의 현재 위치와 방향
	FVector StartLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	
	// 공격 범위 계산 (앞쪽으로 Range만큼)
	FVector EndLocation = StartLocation + (ForwardVector * Range);
	
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
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeCapsule(CapsuleAttackRadius, CapsuleAttackHeight),
		QueryParams
	);


	FColor DebugColor = bHit ? FColor::Red : FColor::Green;
	DrawDebugCapsule(GetWorld(), (StartLocation + EndLocation) / 2, CapsuleAttackHeight, CapsuleAttackRadius,
		FQuat::Identity, DebugColor, false, 2.0f);


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
					Player->TakeCharacterDamage(AttackDamage);
					UE_LOG(LogTemp, Warning, TEXT("Dragon hit player for %d damage"), AttackDamage);
				}
			}
		}
	}
}

void ARLCharacterEnemyDragon::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 현재 실행 중인 몽타주가 끝나면 CurrentMontage를 null로 설정
	if (CurrentMontage == Montage)
	{
		CurrentMontage = nullptr;
	}
	
	// 죽음 몽타주가 끝나면 특별한 처리는 하지 않음 (이미 타이머로 Destroy 설정됨)
}

