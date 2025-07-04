// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLStageClearPortalWidget.h"
#include "Components/Button.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"


URLStageClearPortalWidget::URLStageClearPortalWidget(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// 키 입력 받아올지 설정
	bIsFocusable = true;
}

void URLStageClearPortalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	uint8 StageExit = Player->GetbStageExit();
	if (StageExit == 0)
	{
		NextStageButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		NextStageButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	GoHomeButton->OnClicked.AddDynamic(this, &URLStageClearPortalWidget::ClearSatgeExit);
	NextStageButton->OnClicked.AddDynamic(this, &URLStageClearPortalWidget::NextStage);
	ExitStageButton->OnClicked.AddDynamic(this, &URLStageClearPortalWidget::NotClearStageExit);

}

// 플레이어 키 입력을 받아옴
FReply URLStageClearPortalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	// ESC 키에 대한 처리
	if (PressedKey == EKeys::Escape)
	{
		// 위젯 종료
		NotClearStageExit();

		// 입력 변환
		return FReply::Handled();
	}

	// 다른 키 입력 시 처리
	return FReply::Unhandled();
}

// 클리어되어 나가는 버튼
void URLStageClearPortalWidget::ClearSatgeExit()
{
	// 타운으로 이동
	UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// 다음 스테이지로 버튼
void URLStageClearPortalWidget::NextStage()
{
	// 다음 스테이지로 이동 (현재 밸런싱중)
	UGameplayStatics::OpenLevel(GetWorld(), "BattleStage1");
}

void URLStageClearPortalWidget::NotClearStageExit()
{
    // 일시정지 해제
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    // 마우스 커서 숨기기
    PlayerController->SetShowMouseCursor(false);

    // 위젯 종료
    RemoveFromParent();

    // 게임만 입력만 받기
    FInputModeGameOnly InputMode;
    PlayerController->SetInputMode(InputMode);
}



