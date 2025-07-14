// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLPlayerUI.h"
#include "Character/RLCharacterPlayer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void URLPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 필요한 플레이어 찾기
	Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	UpdatePlayerMoney();
}

void URLPlayerUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

void URLPlayerUI::PlayerCalculateHp(float NewCurrentHp, float NewMaxHp)
{

	if (!PlayerHpBar) return;
	float Ratio = NewCurrentHp / NewMaxHp;
	PlayerHpBar->SetPercent(Ratio);
}

void URLPlayerUI::SkillCoolTime(uint8 CoolCheck)
{
	bCanSkill = CoolCheck;
}

void URLPlayerUI::UpdatePlayerMoney()
{
	MoneyData->SetText(FText::AsNumber(Player->GetMoney()));
}



