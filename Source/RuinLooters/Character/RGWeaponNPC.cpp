// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RGWeaponNPC.h"
#include "GameInstance/RGGameInstance.h"
#include "Controller/RGPlayerController.h"
#include "Character/RGCharacterPlayer.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "UI/NPCStoreWidget.h"

void ARGWeaponNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGNPC::OnDetectPlayerBoxBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// NPC�� ��ȣ�ۿ� ���� �� �� ���� ����
	if (!ActiveStoreWidget && StoreClass && PlayerController)
	{
		// CreateWidget: PlayerController �� WorldContext�� �Ѱܾ� �մϴ�
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

	CastedStoreWiget->OnBuyPressed.AddUObject(this, &IRGNPCBuyInterface::HandleStoreBuy);
}

void ARGWeaponNPC::HandleStoreBuy()
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
