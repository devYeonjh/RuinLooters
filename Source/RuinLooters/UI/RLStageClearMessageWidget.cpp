// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLStageClearMessageWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"

void URLStageClearMessageWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	World = GetWorld();
	
	Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));

	Player->SetMoney(Player->GetMoney() + 2000);

	// 3�� �� widget ����
	World->GetTimerManager().SetTimer(ViewClearTextTimer, this, &URLStageClearMessageWidget::CloseSelf, 3.0f, false);

	GameClear.Broadcast();
}

void URLStageClearMessageWidget::CloseSelf()
{
	RemoveFromParent();
}



