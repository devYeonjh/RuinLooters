// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLPlayerUI.h"
#include "Character/RLCharacterPlayer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void URLPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 필요한 플레이어 찾기
	Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	UpdatePlayerMoney();

	HideArrowPoint();
	HideArrowCount(); // 기본적으로 화살 갯수 숨김
	HideArrowIcon(); // 기본적으로 화살 아이콘 숨김
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

void URLPlayerUI::ShowArrowPoint()
{
	if (ArrowPoint)
	{
		ArrowPoint->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("ArrowPoint shown"));
	}
}

void URLPlayerUI::HideArrowPoint()
{
	if (ArrowPoint)
	{
		ArrowPoint->SetVisibility(ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Log, TEXT("ArrowPoint hidden"));
	}
}

void URLPlayerUI::UpdateArrowCount(int32 CurrentCount, int32 MaxCount)
{
	if (ArrowCountText)
	{
		FText ArrowCountFormat = FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentCount, MaxCount));
		ArrowCountText->SetText(ArrowCountFormat);
		
		// 화살 갯수에 따른 색상 변경
		if (CurrentCount <= 0)
		{
			// 빨간색 (화살이 없을 때)
			ArrowCountText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
		else
		{
			// 하얀색 (기본 색상)
			ArrowCountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		
		UE_LOG(LogTemp, Log, TEXT("Arrow count updated: %d/%d"), CurrentCount, MaxCount);
	}
}

void URLPlayerUI::ShowArrowCount()
{
	if (ArrowCountText)
	{
		ArrowCountText->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("Arrow count shown"));
	}
}

void URLPlayerUI::HideArrowCount()
{
	if (ArrowCountText)
	{
		ArrowCountText->SetVisibility(ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Log, TEXT("Arrow count hidden"));
	}
}

void URLPlayerUI::ShowArrowIcon()
{
	if (ArrowIcon)
	{
		ArrowIcon->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("Arrow icon shown"));
	}
}

void URLPlayerUI::HideArrowIcon()
{
	if (ArrowIcon)
	{
		ArrowIcon->SetVisibility(ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Log, TEXT("Arrow icon hidden"));
	}
}
