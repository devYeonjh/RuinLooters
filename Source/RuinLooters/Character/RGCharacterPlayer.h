// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RGCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "Containers/Array.h"
#include "RGCharacterPlayer.generated.h"

/**
 * 
 */

// 캐릭터 체력 변동 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerCalculateHp, int32 /*CurrentHp*/, int32 /*MaxHp*/);

// 캐릭터 스킬 쿨타임 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FSkillCoolTime, uint8 /*CoolCheck*/);

UCLASS()
class RUINLOOTERS_API ARGCharacterPlayer : public ARGCharacterBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ARGCharacterPlayer();

	uint8 bIsCharacterInteractWithNPC : 1;

	// PlayerUI Widget -> Hp, SkillCool
	FPlayerCalculateHp PlayerHpChange;
	FSkillCoolTime SkillCoolChange;


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

	// NPC
	UPROPERTY()
	class ARGNPC* InteractiveNPC;

	// 플레이어 체력 및 스킬 쿨타임
	UPROPERTY()
	class URGPlayerUI* PlayerUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URGPlayerUI> PlayerUIClass;

	// 설정창
	UPROPERTY()
	class URGSettingsMenuWidget* SettingsWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URGSettingsMenuWidget> SettingsWidgetClass;

	// 사망 UI
	UPROPERTY()
	class URGPlayerDeadWidget* PlayerDieUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URGPlayerDeadWidget> PlayerDieUIClass;

	// 플레이어는 0번 팀, 적은 1번 팀
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 TeamID = 0;

	UPROPERTY()
	class APlayerController* PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	class URGPlayerDataAsset* PlayerStat;

	// n번째 스테이지로 n이 커질수록 난이도가 올라감
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 StageIndex;

	// 현재 Level Name
	UPROPERTY()
	FName LevelName;

	// 살아있는 적 숫자
	uint32 WorldAliveEnemys;

	// 적 존재하는지를 확인
	uint8 bStageExit : 1;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URGStageClearMessageWidget> StageClearWidgetClass;

	UPROPERTY()
	class URGStageClearMessageWidget* StageClearMessageUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URGStageClearPortalWidget> StagePortalWidgetClass;

	UPROPERTY()
	class URGStageClearPortalWidget* StagePortalUI;

	UPROPERTY()
	class URGPlayerDataAsset* LoadAsset;

	// 원래 속도를 보관
	float OriginalMaxWalkSpeed;

	// 버프 만료를 관리하는 핸들
	FTimerHandle SpeedBuffTimerHandle;

	FTimerHandle CoolTimerHandle;

	uint8 IsCanSkill : 1;

public:
	FORCEINLINE ARGCharacterPlayer* GetPlayer() { return this; };
	FORCEINLINE class ARGNPC* GetInteractNPC() { return InteractiveNPC; };
	FORCEINLINE void SetInteractNPC(class ARGNPC* NewNPC) { InteractiveNPC = NewNPC; };
	FORCEINLINE UInputMappingContext* GetDefaultMappingContext() { return DefaultMappingContext; };
	FORCEINLINE class URGPlayerDataAsset* GetPlayerStat() { return PlayerStat; };
	FORCEINLINE void SetLevelName(FName NewLevelName) { LevelName = NewLevelName; };
	FORCEINLINE uint32 GetWorldAliveEnemyCount() { return WorldAliveEnemys; };
	FORCEINLINE void SubtractionWorldAliveEnemyCount() { WorldAliveEnemys--; };
	FORCEINLINE FName GetLevelName() { return LevelName; };
	FORCEINLINE uint8 GetbStageExit() { return bStageExit; };
	FORCEINLINE void SetbStageExit(bool UpdatebStageExit) { bStageExit = UpdatebStageExit; };
	FORCEINLINE const FTimerHandle& GetCoolTimerHandle() const { return CoolTimerHandle; }
	FORCEINLINE void StageIndexUp() { StageIndex++; };

	// 인터페이스용 메서드 오버라이드
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void TakeCharacterDamage(int32 RecieveDamage) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount) override;

	void TakeCharacterMaxHealth(int32 UpScale);

	void TakeCharacterDefence(int32 UpScale);

	void PrintMoney();

	// ESC 바인딩
	void ViewSettingWidget();

	void GetSaveGame();

	void ShowStageClearWidget();

	void ShowStagePortalWidget();

protected:
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Die() override;

	// NPC와 인터렉션
	void Interaction();

	void ApplySpeedBuff();

	// 타이머 만료 시 원래 속도로 복구
	void RestoreOriginalSpeed();

	void OnSkill();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetPlayerStat();

	uint8 CheckEnemy();
};
