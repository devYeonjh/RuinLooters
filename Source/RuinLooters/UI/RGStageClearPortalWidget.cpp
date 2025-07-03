// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RGStageClearPortalWidget.h"
#include "Components/Button.h"
#include "Character/RGCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"


URGStageClearPortalWidget::URGStageClearPortalWidget(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Ű �Է� �޾ƿ��°� ���
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

// ������ Ű �Է°� �޾ƿ�
FReply URGStageClearPortalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	// ESC ������ ��
	if (PressedKey == EKeys::Escape)
	{
		// ���� ����
		NotClearStageExit();

		// �Է� ��ȯ
		return FReply::Handled();
	}

	// �ٸ� Ű �Է� �� ����
	return FReply::Unhandled();
}

// �������� ������ ��ư
void URGStageClearPortalWidget::ClearSatgeExit()
{
	// ������ �̵�
	UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// ���� �������� ��ư
void URGStageClearPortalWidget::NextStage()
{
	// ���� ���������� �̵� (���� ��������)
	UGameplayStatics::OpenLevel(GetWorld(), "BattleStage1");
}

void URGStageClearPortalWidget::NotClearStageExit()
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
