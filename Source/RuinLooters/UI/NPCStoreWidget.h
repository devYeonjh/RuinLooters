// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NPCStoreWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnBuyPressed);

/**
 * 
 */
UCLASS()
class RUINLOOTERS_API UNPCStoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnBuyPressed OnBuyPressed;

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* ItemImage;

	UPROPERTY(meta = (BindWidget))
	class UButton* BuyButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* DeleteButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemPrice;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerMoney;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemNameText;

	UPROPERTY()
	class ARGCharacterPlayer* Player;

public:
	FORCEINLINE class UImage* GetItemImage() { return ItemImage; };
	FORCEINLINE class UButton* GetBuyButton() { return BuyButton; };
	FORCEINLINE class UTextBlock* GetItemPrice() { return ItemPrice; };
	FORCEINLINE class UTextBlock* GetPlayerMoney() { return PlayerMoney; };
	FORCEINLINE class UTextBlock* GetItemNameText() { return ItemNameText; };


	virtual void NativeConstruct() override;

	UFUNCTION()
	void BuyItem();

	UFUNCTION()
	void DeleteWidget();

};
