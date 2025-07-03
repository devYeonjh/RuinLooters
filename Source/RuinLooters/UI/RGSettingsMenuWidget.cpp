// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RGSettingsMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RGCharacterPlayer.h"

URGSettingsMenuWidget::URGSettingsMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bIsFocusable = true;
}

void URGSettingsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ExitButton->OnClicked.AddDynamic(this, &URGSettingsMenuWidget::ExitSatge);
    ConfButton->OnClicked.AddDynamic(this, &URGSettingsMenuWidget::CloaseWidget);
    QuitButton->OnClicked.AddDynamic(this, &URGSettingsMenuWidget::QuitGame);

    Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

// 위젯의 키 입력값 받아옴
FReply URGSettingsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    const FKey PressedKey = InKeyEvent.GetKey();

    // ESC 눌렸을 때
    if (PressedKey == EKeys::Escape)
    {
        // 위젯 제거
        CloaseWidget();

        // 입력 반환
        return FReply::Handled();
    }

    // 다른 키 입력 시 무시
    return FReply::Unhandled();
}

void URGSettingsMenuWidget::CheckLevelName(FName NewLevelName)
{
	// 스테이지가 아니면 위젯 버튼 숨김
	if (NewLevelName.ToString().Contains(TEXT("Stage")))
	{
		ExitButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ExitButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// stage 나가기 버튼
void URGSettingsMenuWidget::ExitSatge()
{
    Player->SetbStageExit(true);

    // 마을로 이동
    UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// 취소 버튼(ConfButton) OnClicked 이벤트 함수 => 위젯 제거
void URGSettingsMenuWidget::CloaseWidget()
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

// 나가기 버튼(QuitButton) OnClicked 이벤트 함수 => 게임 종료
void URGSettingsMenuWidget::QuitGame()
{
    UWorld* CurrentWorld = GetWorld();

    Player->SetbStageExit(true);

    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);

}