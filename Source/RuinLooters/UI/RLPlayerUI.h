// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RLPlayerUI.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URLPlayerUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* MoneyData;

protected:
	int8 bCanSkill : 1;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* PlayerHpBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* SkillCoolBar;

	UPROPERTY(meta = (BindWidget))
	class UImage* ArrowPoint;
	
	// 화살 갯수 표시 UI
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ArrowCountText;
	
	// 화살 아이콘 이미지
	UPROPERTY(meta = (BindWidget))
	class UImage* ArrowIcon;

	UPROPERTY()
	class ARLCharacterPlayer* Player;

public:
	void PlayerCalculateHp(float NewCurrentHp, float NewMaxHp);
	void SkillCoolTime(uint8 CoolCheck);
	void UpdatePlayerMoney();
	
	// 화살 조준점 제어
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void ShowArrowPoint();
	
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void HideArrowPoint();
	
	// 화살 갯수 업데이트
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void UpdateArrowCount(int32 CurrentCount, int32 MaxCount);
	
	// 화살 갯수 UI 표시/숨기기
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void ShowArrowCount();
	
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void HideArrowCount();
	
	// 화살 아이콘 표시/숨기기
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void ShowArrowIcon();
	
	UFUNCTION(BlueprintCallable, Category = "Arrow UI")
	void HideArrowIcon();
};



