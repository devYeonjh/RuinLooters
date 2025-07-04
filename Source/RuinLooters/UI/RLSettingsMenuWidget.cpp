// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLSettingsMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"

URLSettingsMenuWidget::URLSettingsMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bIsFocusable = true;
}

void URLSettingsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ExitButton->OnClicked.AddDynamic(this, &URLSettingsMenuWidget::ExitSatge);
    ConfButton->OnClicked.AddDynamic(this, &URLSettingsMenuWidget::CloaseWidget);
    QuitButton->OnClicked.AddDynamic(this, &URLSettingsMenuWidget::QuitGame);

    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

// 플레이어 키 입력을 받아옴
FReply URLSettingsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    const FKey PressedKey = InKeyEvent.GetKey();

    // ESC 키에 대한 처리
    if (PressedKey == EKeys::Escape)
    {
        // 위젯 종료
        CloaseWidget();

        // 입력 변환
        return FReply::Handled();
    }

    // 다른 키 입력 시 처리
    return FReply::Unhandled();
}

void URLSettingsMenuWidget::CheckLevelName(FName NewLevelName)
{
	// 스테이지가 아니면 나가기 버튼 숨김
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
void URLSettingsMenuWidget::ExitSatge()
{
    Player->SetbStageExit(true);

    // 타운으로 이동
    UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// 확인 버튼(ConfButton) OnClicked 이벤트 함수 => 위젯 종료
void URLSettingsMenuWidget::CloaseWidget()
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

// 종료버튼(QuitButton) OnClicked 이벤트 함수 => 게임 종료
void URLSettingsMenuWidget::QuitGame()
{
    UWorld* CurrentWorld = GetWorld();

    Player->SetbStageExit(true);

    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);

}


