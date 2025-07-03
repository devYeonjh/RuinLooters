// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RGPlayerDeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGPlayerDeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	class ARGCharacterPlayer* Player;

protected:
	UFUNCTION()
	void QuitGame();

	UFUNCTION()
	void RestartGame();

};
