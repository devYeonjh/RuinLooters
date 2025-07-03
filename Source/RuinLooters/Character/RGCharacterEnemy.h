// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RGCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "RGCharacterEnemy.generated.h"

// �� ü�� ���� ��������Ʈ
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHpChange, int32 /*CurrentHp*/, int32 /*MaxHp*/);

/**
 * 
 */

UCLASS()
class RUINLOOTERS_API ARGCharacterEnemy : public ARGCharacterBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	ARGCharacterEnemy();

	FOnHpChange EnemyHpChange;

	struct FEnemyAbilityTableRow* EnemyAbilityRow;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player")
	class ARGCharacterPlayer* Player;

	// Enemy ���� Hp ����
	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<class UHPWidget> HpWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* HealthBarComponent;

	// �÷��̾�� 0�� ��, ���� 1�� ��
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 TeamID = 1;

	// ������ ���̺����� �� ���� �����ϱ� ���� ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FName EnemyName;


protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE FName GetEnemyName() { return EnemyName; };
	FORCEINLINE void SetEnemyName(FName NewEnemyName) { EnemyName = NewEnemyName; };

	// ĳ���� �� ��ȯ
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void TakeCharacterDamage(int32 RecieveDamage) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount) override;

protected:
	virtual void Die() override;

	void DestoryCharacter();

	FTimerHandle DieTimerHandle;
};
