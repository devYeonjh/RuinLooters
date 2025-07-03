// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RGPlayerUI.h"
#include "Character/RGCharacterPlayer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void URGPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 맵에서 플레이어 찾기
	Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	UpdatePlayerMoney();
}

void URGPlayerUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!SkillCoolBar) return;
	float RemainingTime = 0.0f;
	if (Player)
	{

		RemainingTime = Player->GetWorld()->GetTimerManager().GetTimerRemaining(Player->GetCoolTimerHandle());
		//UE_LOG(LogTemp, Warning, TEXT("RemainingTime : %f"), RemainingTime/8);
	}

	if (!bCanSkill) {
		SkillCoolBar->SetPercent(RemainingTime / 8);
	}
}

void URGPlayerUI::PlayerCalculateHp(int32 NewCurrentHp, int32 NewMaxHp)
{

	if (!PlayerHpBar) return;
	float Ratio = (float)NewCurrentHp / NewMaxHp;
	PlayerHpBar->SetPercent(Ratio);
}

void URGPlayerUI::SkillCoolTime(uint8 CoolCheck)
{
	bCanSkill = CoolCheck;
}

void URGPlayerUI::UpdatePlayerMoney()
{
	MoneyData->SetText(FText::AsNumber(Player->GetMoney()));
}
