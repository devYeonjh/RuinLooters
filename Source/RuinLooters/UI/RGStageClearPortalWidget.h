// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RGStageClearPortalWidget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGStageClearPortalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URGStageClearPortalWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* GoHomeButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* NextStageButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ExitStageButton;

	class ARGCharacterPlayer* Player;


protected:
	UFUNCTION()
	void ClearSatgeExit();

	UFUNCTION()
	void NextStage();

	UFUNCTION()
	void NotClearStageExit();
};
