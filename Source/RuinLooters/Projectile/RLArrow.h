// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "RLArrow.generated.h"

/**
 * 플레이어가 사용하는 화살 클래스
 * 독립적인 AActor로서 스태틱 메시와 ProjectileMovement만 가진 단순한 구현
 */
UCLASS()
class ARLArrow : public AActor
{
	GENERATED_BODY()

public:
	ARLArrow();

protected:
	virtual void BeginPlay() override;

	// 스피어 콜리전 컴포넌트 (오버랩 감지용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereCollision;

	// 스태틱 메시 컴포넌트 (화살 모델)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ArrowMesh;

	// 투사체 이동 컴포넌트 (발사될 때만 활성화)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 화살 기본 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Settings")
	int32 Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Settings")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Settings")
	float LifeTime;

	// 화살 발사 방향 조정 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Settings")
	FRotator ArrowDirectionOffset;

	// 파티클 이팩트 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
	class UParticleSystem* TrailParticleTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
	class UParticleSystem* HitParticleTemplate;

	// 파티클 위치 오프셋 (블루프린트에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
	FVector TrailParticleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
	FVector HitParticleOffset;

public:
	// 화살 초기화 (카메라 기준 목표 위치 사용)
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void InitializeArrow(FVector StartLocation, class APlayerController* PlayerController, int32 ArrowDamage = 30, float ArrowSpeed = 2500.0f);
	
	// 소켓에 부착용 함수
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void AttachToSocket(USkeletalMeshComponent* TargetMesh, FName SocketName);
	
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void DetachFromSocket();

	// 화살 비활성화 (풀링용)
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void DeactivateArrow();

	// 화살 활성화 (풀링용)
	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void ActivateArrow();

	// 오버랩 처리
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	// 소켓 부착 상태
	bool bIsAttachedToSocket;
	
	// 부착된 컴포넌트 참조
	UPROPERTY()
	USkeletalMeshComponent* AttachedMeshComponent;
	
	FName AttachedSocketName;

	// 라이프타임 타이머
	FTimerHandle LifeTimeHandle;

	// 파티클 시스템 컴포넌트들
	UPROPERTY()
	class UParticleSystemComponent* TrailParticleComponent;

	// 라이프타임 만료 처리
	void OnLifeTimeExpired();

	// 파티클 이펙트 생성
	void CreateTrailParticle();
	void CreateHitParticle(FVector HitLocation);
}; 