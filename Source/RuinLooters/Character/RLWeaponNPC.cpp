// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLWeaponNPC.h"
#include "GameInstance/RLGameInstance.h"
#include "Controller/RLPlayerController.h"
#include "Character/RLCharacterPlayer.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "UI/NPCStoreWidget.h"

void ARLWeaponNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARLNPC::OnDetectPlayerBoxBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// NPC와 상호작용할 때만 한 번 위젯 생성
	if (!ActiveStoreWidget && StoreClass && PlayerController)
	{
		// CreateWidget: PlayerController 를 WorldContext로 넘겨야 합니다
		if (World && GameInstance)
		{
			Weapon = GameInstance->GetRamdomWeapon();
		}

		ActiveStoreWidget = CreateWidget<UUserWidget>(PlayerController, StoreClass);
		if (ActiveStoreWidget)
		{
			CastedStoreWiget = Cast<UNPCStoreWidget>(ActiveStoreWidget);

			CastedStoreWiget->GetItemImage()->SetBrushFromTexture(Weapon->Icon, /*bMatchSize=*/ true);
			CastedStoreWiget->GetItemPrice()->SetText(FText::AsNumber(Weapon->Price));
			CastedStoreWiget->GetItemNameText()->SetText(FText::FromName(GameInstance->GetWeaponName(Weapon->WeaponIndex)));
		}
	}

	CastedStoreWiget->GetPlayerMoney()->SetText(FText::AsNumber(Player->GetMoney()));

	CastedStoreWiget->OnBuyPressed.AddUObject(this, &IRLNPCBuyInterface::HandleStoreBuy);
}

void ARLWeaponNPC::HandleStoreBuy()
{
	if (Player->GetMoney() >= Weapon->Price)
	{
		Player->SetMoney(Player->GetMoney() - Weapon->Price);
		Player->ChangeWeapon(Weapon);
		CastedStoreWiget->GetPlayerMoney()->SetText(FText::AsNumber(Player->GetMoney()));
		Player->PrintMoney();
		UE_LOG(LogTemp, Warning, TEXT("Buy item, left money: %d"), Player->GetMoney());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Enough Money"));
	}
}



