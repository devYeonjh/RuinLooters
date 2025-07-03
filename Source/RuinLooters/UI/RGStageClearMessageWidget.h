// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RGStageClearMessageWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnClearGame)

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API URGStageClearMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	FOnClearGame GameClear;

protected:
	UFUNCTION()
	void CloseSelf();

	FTimerHandle ViewClearTextTimer;

	UPROPERTY()
	class UWorld* World;

	UPROPERTY()
	class ARGCharacterPlayer* Player;
};
