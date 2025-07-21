// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RLCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "Containers/Array.h"
#include "RLGliderComponent.h"
#include "RLCharacterPlayer.generated.h"

/**
 * 
 */

// 캐릭터 체력 변화 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerCalculateHp, float /*CurrentHp*/, float /*MaxHp*/);

// 캐릭터 스킬 쿨타임 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FSkillCoolTime, uint8 /*CoolCheck*/);

// 에이밍 상태 변화 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FAimingStateChanged, bool /*bIsAiming*/);

// 화살 갯수 변화 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FArrowCountChanged, int32 /*CurrentCount*/, int32 /*MaxCount*/);

UCLASS()
class RUINLOOTERS_API ARLCharacterPlayer : public ARLCharacterBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ARLCharacterPlayer();

	uint8 bIsCharacterInteractWithNPC : 1;

	// PlayerUI Widget -> Hp, SkillCool, Aiming, Arrow
	FPlayerCalculateHp PlayerHpChange;
	FSkillCoolTime SkillCoolChange;
	FAimingStateChanged AimingStateChanged;
	FArrowCountChanged ArrowCountChanged;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Glider", meta = (AllowPrivateAccess = "true"))
	URLGliderComponent* GliderComponent;

public:
	float CameraTargetArmLength = 400.0f; // 카메라 목표 거리(기본값)

protected:
	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractionAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SettingsAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ProjectileSkillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* FormChangeAction;

	// 투사체 스킬 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* ProjectileSkillMontage;

	// 스피드 스킬 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* SpeedSkillMontage;

	// 활 시위를 당기는 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* BowDrawMontage;

	// NPC
	UPROPERTY()
	class ARLNPC* InteractiveNPC;

	// 플레이어 체력 및 스킬 쿨타임
	UPROPERTY()
	class URLPlayerUI* PlayerUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLPlayerUI> PlayerUIClass;

	// 설정창
	UPROPERTY()
	class URLSettingsMenuWidget* SettingsWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLSettingsMenuWidget> SettingsWidgetClass;

	// 죽음 UI
	UPROPERTY()
	class URLPlayerDeadWidget* PlayerDieUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLPlayerDeadWidget> PlayerDieUIClass;

	// 플레이어는 0번 팀, 적은 1번 팀
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 TeamID = 0;

	UPROPERTY()
	class APlayerController* PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	class URLPlayerDataAsset* PlayerStat;

	// n번째 스테이지일 때 n번 커서에서 스테이지가 열림
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 StageIndex;

	// 현재 Level Name
	UPROPERTY()
	FName LevelName;

	// 살아있는 적 개수
	uint32 WorldAliveEnemys;

	// 스테이지 나가는지 확인
	uint8 bStageExit : 1;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLStageClearMessageWidget> StageClearWidgetClass;

	UPROPERTY()
	class URLStageClearMessageWidget* StageClearMessageUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLStageClearPortalWidget> StagePortalWidgetClass;

	UPROPERTY()
	class URLStageClearPortalWidget* StagePortalUI;

	UPROPERTY()
	class URLPlayerDataAsset* LoadAsset;

	// 스킬 버프를 관리하는 핸들
	FTimerHandle SpeedBuffTimerHandle;

	FTimerHandle CoolTimerHandle;

	uint8 IsCanSkill : 1;

	// 투사체 스킬 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Projectile")
	TSubclassOf<class ARLProjectile> PlayerProjectileClass;

	UPROPERTY()
	class URLProjectilePool* PlayerProjectilePool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Projectile")
	int32 ProjectileDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Projectile")
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Projectile")
	int32 ProjectileSkillCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Projectile")
	float ProjectileCollisionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Projectile")
	float ProjectileCollisionHeight;

	UPROPERTY()
	uint8 bCanUseProjectileSkill : 1;

	FTimerHandle ProjectileSkillCooldownHandle;

	// 투사체 스킬 중 이동 제한 관련
	UPROPERTY()
	uint8 bIsUsingProjectileSkill : 1;

	// 투사체 스킬 사용 시 원래 속도 저장
	float OriginalWalkSpeedForProjectile;

	// 안전 타이머 핸들
	FTimerHandle SafetyTimerHandle;
	
	// 화살 차징 타이머 핸들
	FTimerHandle ChargingTimerHandle;
	
	// 화살 갯수 시스템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow")
	int32 CurrentArrowCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	int32 MaxArrowCount;

public:
	FORCEINLINE ARLCharacterPlayer* GetPlayer() { return this; };
	FORCEINLINE class ARLNPC* GetInteractNPC() { return InteractiveNPC; };
	FORCEINLINE void SetInteractNPC(class ARLNPC* NewNPC) { InteractiveNPC = NewNPC; };
	FORCEINLINE UInputMappingContext* GetDefaultMappingContext() { return DefaultMappingContext; };
	FORCEINLINE class URLPlayerDataAsset* GetPlayerStat() { return PlayerStat; };
	FORCEINLINE void SetLevelName(FName NewLevelName) { LevelName = NewLevelName; };
	FORCEINLINE uint32 GetWorldAliveEnemyCount() { return WorldAliveEnemys; };
	FORCEINLINE void SubtractionWorldAliveEnemyCount() { WorldAliveEnemys--; };
	FORCEINLINE FName GetLevelName() { return LevelName; };
	FORCEINLINE uint8 GetbStageExit() { return bStageExit; };
	FORCEINLINE void SetbStageExit(bool UpdatebStageExit) { bStageExit = UpdatebStageExit; };
	FORCEINLINE const FTimerHandle& GetCoolTimerHandle() const { return CoolTimerHandle; }
	FORCEINLINE void StageIndexUp() { StageIndex++; };
	FORCEINLINE bool GetbIsSword() { return bIsSword; };
	FORCEINLINE bool GetbIsAiming() { return bIsAiming; };
	FORCEINLINE int32 GetCurrentArrowCount() { return CurrentArrowCount; };
	FORCEINLINE int32 GetMaxArrowCount() { return MaxArrowCount; };

	// 인터페이스의 메서드 오버라이드
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount) override;

	void TakeCharacterMaxHealth(int32 UpScale);

	void TakeCharacterDefence(int32 UpScale);

	void PrintMoney();

	// ESC 바인딩
	void ViewSettingWidget();

	void GetSaveGame();

	void ShowStageClearWidget();

	void ShowStagePortalWidget();

	// 투사체 스킬 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Player Projectile")
	void UseProjectileSkill();

	// 투사체 스킬 몽타주 종료 콜백
	UFUNCTION()
	void OnProjectileSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 스피드 스킬 몽타주 종료 콜백
	UFUNCTION()
	void OnSpeedSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 스피드 버프 효과 적용 함수
	void ApplySpeedBuffEffect();

	UFUNCTION(BlueprintCallable, Category = "Player Projectile")
	void FirePlayerProjectile();
	void HandleJumpOrGlide();
	virtual void StartRoll();
	virtual void Attack() override;

	// 플레이어 입력 차단/복원 함수 오버라이드
	virtual void DisablePlayerInput() override;
	virtual void EnablePlayerInput() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Die() override;

	// NPC와 상호작용
	void Interaction();

	void ApplySpeedBuff();

	// 타이머 끝난 후 원래 속도로 복귀
	void RestoreOriginalSpeed();

	void OnSkill();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetPlayerStat();

	uint8 CheckEnemy();

	FOnMontageEnded MontageEndedDelegate;

	// 에이밍 시스템 관련 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aiming")
	bool bIsAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming") 
	float AimingCameraDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	float NormalCameraDistance;

	// 폼 체인지 관련 변수 (검/활 전환)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Form")
	bool bIsSword;

	// 에이밍 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	class USoundBase* BowDrawSound;

	// 활 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bow")
	class USkeletalMeshComponent* BowMeshComponent;

	// 화살 시스템 관련
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow")
	bool bIsLoadingArrow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow")
	bool bIsArrowLoaded;
	
	// 화살 차징 시스템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow")
	bool bIsChargingArrow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow")
	float CurrentChargeTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float MaxChargeTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float MinArrowSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float MaxArrowSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TSubclassOf<class ARLArrow> ArrowClass;
	
	// 화살 데미지 스케일링 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	int32 MinArrowDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	int32 MaxArrowDamage;
	
	// 화살 풀링을 위한 변수
	UPROPERTY()
	class URLArrowPool* ArrowPool;
	
	UPROPERTY()
	class ARLArrow* LoadedArrow;
	
	// 카메라 관련 변수 (에이밍용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FVector NormalCameraPosition;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FVector AimingCameraPosition;

	// 에이밍 시스템 관련 함수
	UFUNCTION()
	void StartAiming();
	
	UFUNCTION()
	void StopAiming();

	// 폼 체인지 (검/활 전환)
	UFUNCTION()
	void ChangeForm();

	// 화살 시스템 관련 함수
	UFUNCTION()
	void StartLoadingArrow();
	
	UFUNCTION()
	void StopChargingArrow();
	
	UFUNCTION()
	void FireArrow();
	
	UFUNCTION()
	void LoadArrowToSocket();
	
	// 차징 시스템 관련 함수
	void UpdateCharging(float DeltaTime);
	float CalculateArrowSpeed() const;
	
	// 데미지 계산 함수
	int32 CalculateArrowDamage() const;
	


	// 공격 버튼 홀딩 시스템
	UFUNCTION()
	void OnAttackPressed();
	
	UFUNCTION()
	void OnAttackReleased();

	virtual void CallAttackCollision() override;
};



