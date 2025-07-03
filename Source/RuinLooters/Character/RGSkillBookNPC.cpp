// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RGSkillBookNPC.h"
#include "GameInstance/RGGameInstance.h"
#include "Controller/RGPlayerController.h"
#include "Character/RGCharacterPlayer.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "UI/NPCStoreWidget.h"

void ARGSkillBookNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGNPC::OnDetectPlayerBoxBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// NPC와 상호작용 시작 시 한 번만 띄우기
	if (!ActiveStoreWidget && StoreClass && PlayerController)
	{
		// CreateWidget: PlayerController 를 WorldContext로 넘겨야 합니다
		if (World && GameInstance)
		{
			SkillBook = GameInstance->GetRamdomSkillBook();
		}

		ActiveStoreWidget = CreateWidget<UUserWidget>(PlayerController, StoreClass);
		if (ActiveStoreWidget)
		{
			CastedStoreWiget = Cast<UNPCStoreWidget>(ActiveStoreWidget);

			CastedStoreWiget->GetItemImage()->SetBrushFromTexture(SkillBook->Icon, /*bMatchSize=*/ true);
			CastedStoreWiget->GetItemPrice()->SetText(FText::AsNumber(SkillBook->Price));
			CastedStoreWiget->GetItemNameText()->SetText(FText::FromName(GameInstance->GetSkillBookName(SkillBook->SkillBookIndex)));
		}
	}

	CastedStoreWiget->GetPlayerMoney()->SetText(FText::AsNumber(Player->GetMoney()));

	CastedStoreWiget->OnBuyPressed.AddUObject(this, &IRGNPCBuyInterface::HandleStoreBuy);
}

void ARGSkillBookNPC::HandleStoreBuy()
{
	if (Player->GetMoney() >= SkillBook->Price)
	{
		Player->SetMoney(Player->GetMoney() - SkillBook->Price);
		
		UE_LOG(LogTemp, Warning, TEXT("MaxHp: %d ."), Player->GetMaxHp());
		UE_LOG(LogTemp, Warning, TEXT("defence: %d ."), Player->GetDefence());

		if (SkillBook->SkillBookIndex <= 2)
		{
			Player->TakeCharacterMaxHealth(SkillBook->UpAmount);
			UE_LOG(LogTemp, Warning, TEXT("MaxHp: %d ."), Player->GetMaxHp());
		}
		else
		{
			Player->TakeCharacterDefence(SkillBook->UpAmount);
			UE_LOG(LogTemp, Warning, TEXT("defence: %d ."), Player->GetDefence());
		}

		CastedStoreWiget->GetPlayerMoney()->SetText(FText::AsNumber(Player->GetMoney()));
		Player->PrintMoney();
		UE_LOG(LogTemp, Warning, TEXT("Buy item, left money: %d"), Player->GetMoney());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Enough Money"));
	}
}
