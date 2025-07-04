// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RLPlayerDeadWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"

void URLPlayerDeadWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RestartButton->OnClicked.AddDynamic(this, &URLPlayerDeadWidget::RestartGame);
    QuitButton->OnClicked.AddDynamic(this, &URLPlayerDeadWidget::QuitGame);

    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void URLPlayerDeadWidget::QuitGame()
{
    UWorld* CurrentWorld = GetWorld();

    // ���� ���� �Լ�(����, ��Ʈ�ѷ�, ���� ���? true�� �������� false�� �÷����� �䱸�ϴ� �������?����)
    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void URLPlayerDeadWidget::RestartGame()
{
    // �÷��̾� ü���� 0 �� ��
    // ���� �̵� => EndPlay() ���� => ü�� 0 �� �� => �ʱ�ȭ �� ����
        
    if (Player)
    {
        //  Ÿ������ ���� �̵�
        UGameplayStatics::OpenLevel(GetWorld(), "Town");
    }
}


