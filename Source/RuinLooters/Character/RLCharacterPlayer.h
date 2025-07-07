// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RLCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "Containers/Array.h"
#include "RLCharacterPlayer.generated.h"

/**
 * 
 */

// 캐릭터 체력 변화 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerCalculateHp, int32 /*CurrentHp*/, int32 /*MaxHp*/);

// 캐릭터 스킬 쿨타임 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FSkillCoolTime, uint8 /*CoolCheck*/);

UCLASS()
class RUINLOOTERS_API ARLCharacterPlayer : public ARLCharacterBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ARLCharacterPlayer();

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

	// 인터페이스의 메서드 오버라이드
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

	// NPC와 상호작용
	void Interaction();

	void ApplySpeedBuff();

	// 타이머 끝난 후 원래 속도로 복귀
	void RestoreOriginalSpeed();

	void OnSkill();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetPlayerStat();

	uint8 CheckEnemy();

	// Tick 오버라이드
	// virtual void Tick(float DeltaTime) override;
};



