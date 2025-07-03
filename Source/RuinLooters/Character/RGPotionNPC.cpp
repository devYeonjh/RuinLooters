// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RGPotionNPC.h"
#include "GameInstance/RGGameInstance.h"
#include "Controller/RGPlayerController.h"
#include "Character/RGCharacterPlayer.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "UI/NPCStoreWidget.h"

void ARGPotionNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGNPC::OnDetectPlayerBoxBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// NPC와 상호작용 시작 시 한 번만 띄우기
	if (!ActiveStoreWidget && StoreClass && PlayerController)
	{
		if (World && GameInstance)
		{
			Potion = GameInstance->GetRamdomPotion();
		}

		ActiveStoreWidget = CreateWidget<UUserWidget>(PlayerController, StoreClass);
		if (ActiveStoreWidget)
		{
			CastedStoreWiget = Cast<UNPCStoreWidget>(ActiveStoreWidget);

			// UI 내 텍스트 변경
			CastedStoreWiget->GetItemImage()->SetBrushFromTexture(Potion->Icon, /*bMatchSize=*/ true);
			CastedStoreWiget->GetItemPrice()->SetText(FText::AsNumber(Potion->Price));
			CastedStoreWiget->GetItemNameText()->SetText(FText::FromName(GameInstance->GetPotionName(Potion->PotionIndex)));
		}
	}

	CastedStoreWiget->GetPlayerMoney()->SetText(FText::AsNumber(Player->GetMoney()));

	CastedStoreWiget->OnBuyPressed.AddUObject(this, &IRGNPCBuyInterface::HandleStoreBuy);
}

void ARGPotionNPC::HandleStoreBuy()
{
	if (Player->GetMoney() >= Potion->Price)
	{
		Player->SetMoney(Player->GetMoney() - Potion->Price);
		Player->TakeCharacterHeal(Potion->HealAmount);
		CastedStoreWiget->GetPlayerMoney()->SetText(FText::AsNumber(Player->GetMoney()));
		Player->PrintMoney();
		UE_LOG(LogTemp, Warning, TEXT("Buy item, left money: %d"), Player->GetMoney());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Enough Money"));
	}
}
