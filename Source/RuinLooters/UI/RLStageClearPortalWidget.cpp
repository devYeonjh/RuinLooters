// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLStageClearPortalWidget.h"
#include "Components/Button.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"


URLStageClearPortalWidget::URLStageClearPortalWidget(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Ű �Է� �޾ƿ��°� ���?	bIsFocusable = true;
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

// ������ Ű �Է°� �޾ƿ�
FReply URLStageClearPortalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
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
void URLStageClearPortalWidget::ClearSatgeExit()
{
	// ������ �̵�
	UGameplayStatics::OpenLevel(GetWorld(), "Town");
}

// ���� �������� ��ư
void URLStageClearPortalWidget::NextStage()
{
	// ���� ���������� �̵� (���� ��������)
	UGameplayStatics::OpenLevel(GetWorld(), "BattleStage1");
}

void URLStageClearPortalWidget::NotClearStageExit()
{
    // �Ͻ����� ���� 
    UGameplayStatics::SetGamePaused(GetWorld(), false);

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    // ���콺 Ŀ��  �����?    PlayerController->SetShowMouseCursor(false);

    // ���� ����
    RemoveFromParent();

    // �ΰ��� �Է¸� �ޱ�
    FInputModeGameOnly InputMode;
    PlayerController->SetInputMode(InputMode);
}



