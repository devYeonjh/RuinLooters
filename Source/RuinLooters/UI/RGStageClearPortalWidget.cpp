// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RGStageClearPortalWidget.h"
#include "Components/Button.h"
#include "Character/RGCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"


URGStageClearPortalWidget::URGStageClearPortalWidget(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// 키 입력 받아오는걸 허용
	bIsFocusable = true;
}

void URGStageClearPortalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	uint8 StageExit = Player->GetbStageExit();
	if (StageExit == 0)
	{
		NextStageButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		NextStageButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	GoHomeButton->OnClicked.AddDynamic(this, &URGStageClearPortalWidget::ClearSatgeExit);
	NextStageButton->OnClicked.AddDynamic(this, &URGStageClearPortalWidget::NextStage);
	ExitStageButton->OnClicked.AddDynamic(this, &URGStageClearPortalWidget::NotClearStageExit);

}

// 위젯의 키 입력값 받아옴
FReply URGStageClearPortalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	// ESC 눌렸을 때
	if (PressedKey == EKeys::Escape)
	{
		// 위젯 제거
		NotClearStageExit();

		// 입력 반환
		return FReply::Handled();
	}

	// 다른 키 입력 시 무시
	return FReply::Unhandled();
}

// 스테이지 나가기 버튼
void URGStageClearPortalWidget::ClearSatgeExit()
{
	// 마을로 이동
	UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// 다음 스테이지 버튼
void URGStageClearPortalWidget::NextStage()
{
	// 다음 스테이지로 이동 (동일 스테이지)
	UGameplayStatics::OpenLevel(GetWorld(), "BattleStage1");
}

void URGStageClearPortalWidget::NotClearStageExit()
{
    // 일시정지 해제 
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    // 마우스 커서  숨기기
    PlayerController->SetShowMouseCursor(false);

    // 위젯 제거
    RemoveFromParent();

    // 인게임 입력만 받기
    FInputModeGameOnly InputMode;
    PlayerController->SetInputMode(InputMode);
}
