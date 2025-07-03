// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RGSettingsMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGSettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URGSettingsMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;


protected:

	UPROPERTY()
	class ARGCharacterPlayer* Player;

	UPROPERTY(meta = (BindWidget))
	class UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

public:
	void CheckLevelName(FName NewLevelName);

protected:
	UFUNCTION()
	void ExitSatge();

	UFUNCTION()
	void CloaseWidget();

	UFUNCTION()
	void QuitGame();

};
