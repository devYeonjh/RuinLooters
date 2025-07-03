// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RGStageClearMessageWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RGCharacterPlayer.h"

void URGStageClearMessageWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	World = GetWorld();
	
	Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

	Player->SetMoney(Player->GetMoney() + 2000);

	// 3�� �� widget ����
	World->GetTimerManager().SetTimer(ViewClearTextTimer, this, &URGStageClearMessageWidget::CloseSelf, 3.0f, false);

	GameClear.Broadcast();
}

void URGStageClearMessageWidget::CloseSelf()
{
	RemoveFromParent();
}
