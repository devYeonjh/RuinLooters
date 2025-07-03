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

// ������ Ű �Է°� �޾ƿ�
FReply URGSettingsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    const FKey PressedKey = InKeyEvent.GetKey();

    // ESC ������ ��
    if (PressedKey == EKeys::Escape)
    {
        // ���� ����
        CloaseWidget();

        // �Է� ��ȯ
        return FReply::Handled();
    }

    // �ٸ� Ű �Է� �� ����
    return FReply::Unhandled();
}

void URGSettingsMenuWidget::CheckLevelName(FName NewLevelName)
{
	// ���������� �ƴϸ� ���� ��ư ����
	if (NewLevelName.ToString().Contains(TEXT("Stage")))
	{
		ExitButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ExitButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// stage ������ ��ư
void URGSettingsMenuWidget::ExitSatge()
{
    Player->SetbStageExit(true);

    // ������ �̵�
    UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// ��� ��ư(ConfButton) OnClicked �̺�Ʈ �Լ� => ���� ����
void URGSettingsMenuWidget::CloaseWidget()
{
    // �Ͻ����� ���� 
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    // ���콺 Ŀ��  �����
    PlayerController->SetShowMouseCursor(false);

    // ���� ����
    RemoveFromParent();

    // �ΰ��� �Է¸� �ޱ�
    FInputModeGameOnly InputMode;
    PlayerController->SetInputMode(InputMode);
}

// ������ ��ư(QuitButton) OnClicked �̺�Ʈ �Լ� => ���� ����
void URGSettingsMenuWidget::QuitGame()
{
    UWorld* CurrentWorld = GetWorld();

    Player->SetbStageExit(true);

    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);

}