// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RLCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "RLCharacterEnemy.generated.h"

// 적 체력 변화 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHpChange, float /*CurrentHp*/, float /*MaxHp*/);

/**
 * 
 */

UCLASS()
class RUINLOOTERS_API ARLCharacterEnemy : public ARLCharacterBase, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	ARLCharacterEnemy();

	FOnHpChange EnemyHpChange;

	struct FEnemyAbilityTableRow* EnemyAbilityRow;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player")
	class ARLCharacterPlayer* Player;

	// Enemy 체력 Hp 위젯
	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<class UHPWidget> HpWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UHPWidget> HpWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* HealthBarComponent;

	// 플레이어는 0번 팀, 적은 1번 팀
	UPROPERTY(EditAnywhere, Category = "AI")
	uint8 TeamID = 1;

	// 데이터 테이블에서 적 정보 가져오기 위한 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FName EnemyName;

	// HP 위젯 위치 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector HealthBarLocation;


protected:
	virtual void BeginPlay() override;

	// HP 위젯 설정 함수
	void SetupHealthBarWidget();

public:
	FORCEINLINE FName GetEnemyName() { return EnemyName; };
	FORCEINLINE void SetEnemyName(FName NewEnemyName) { EnemyName = NewEnemyName; };

	// 캐릭터 팀 반환
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void TakeCharacterHeal(int32 RecieveHealAmount) override;

protected:
	virtual void Die() override;

	void DestoryCharacter();

	FTimerHandle DieTimerHandle;

	virtual void CallAttackCollision() override;
};



