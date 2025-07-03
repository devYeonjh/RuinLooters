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

// ĳ���� ü�� ���� ��������Ʈ
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerCalculateHp, int32 /*CurrentHp*/, int32 /*MaxHp*/);

// ĳ���� ��ų ��Ÿ�� ��������Ʈ
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

	// �÷��̾� ü�� �� ��ų ��Ÿ��
	UPROPERTY()
	class URLPlayerUI* PlayerUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLPlayerUI> PlayerUIClass;

	// ����â
	UPROPERTY()
	class URLSettingsMenuWidget* SettingsWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLSettingsMenuWidget> SettingsWidgetClass;

	// ���?UI
	UPROPERTY()
	class URLPlayerDeadWidget* PlayerDieUI;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class URLPlayerDeadWidget> PlayerDieUIClass;

	// �÷��̾��?0�� ��, ���� 1�� ��
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 TeamID = 0;

	UPROPERTY()
	class APlayerController* PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	class URLPlayerDataAsset* PlayerStat;

	// n��° ���������� n�� Ŀ������ ���̵��� �ö�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 StageIndex;

	// ���� Level Name
	UPROPERTY()
	FName LevelName;

	// ����ִ�?�� ����
	uint32 WorldAliveEnemys;

	// �� �����ϴ����� Ȯ��
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

	// ���� �ӵ��� ����
	float OriginalMaxWalkSpeed;

	// ���� ���Ḧ �����ϴ� �ڵ�
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

	// �������̽��� �޼��� �������̵�
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void TakeCharacterDamage(int32 RecieveDamage) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount) override;

	void TakeCharacterMaxHealth(int32 UpScale);

	void TakeCharacterDefence(int32 UpScale);

	void PrintMoney();

	// ESC ���ε�
	void ViewSettingWidget();

	void GetSaveGame();

	void ShowStageClearWidget();

	void ShowStagePortalWidget();

protected:
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Die() override;

	// NPC�� ���ͷ���
	void Interaction();

	void ApplySpeedBuff();

	// Ÿ�̸� ���� �� ���� �ӵ��� ����
	void RestoreOriginalSpeed();

	void OnSkill();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetPlayerStat();

	uint8 CheckEnemy();
};



