// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RGCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "RGCharacterEnemy.generated.h"

// 적 체력 변동 델리게이트
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

	// Enemy 전용 Hp 위젯
	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<class UHPWidget> HpWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* HealthBarComponent;

	// 플레이어는 0번 팀, 적은 1번 팀
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 TeamID = 1;

	// 데이터 테이블에서 적 정보 검출하기 위해 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FName EnemyName;


protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE FName GetEnemyName() { return EnemyName; };
	FORCEINLINE void SetEnemyName(FName NewEnemyName) { EnemyName = NewEnemyName; };

	// 캐릭터 팀 반환
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void TakeCharacterDamage(int32 RecieveDamage) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount) override;

protected:
	virtual void Die() override;

	void DestoryCharacter();

	FTimerHandle DieTimerHandle;
};
