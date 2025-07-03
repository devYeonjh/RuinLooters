// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RGPlayerDeadWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RGCharacterPlayer.h"

void URGPlayerDeadWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RestartButton->OnClicked.AddDynamic(this, &URGPlayerDeadWidget::RestartGame);
    QuitButton->OnClicked.AddDynamic(this, &URGPlayerDeadWidget::QuitGame);

    Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void URGPlayerDeadWidget::QuitGame()
{
    UWorld* CurrentWorld = GetWorld();

    // ���� ���� �Լ�(����, ��Ʈ�ѷ�, ���� ���, true�� �������� false�� �÷����� �䱸�ϴ� ������� ����)
    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void URGPlayerDeadWidget::RestartGame()
{
    // �÷��̾� ü���� 0 �� ��
    // ���� �̵� => EndPlay() ���� => ü�� 0 �� �� => �ʱ�ȭ �� ����
        
    if (Player)
    {
        //  Ÿ������ ���� �̵�
        UGameplayStatics::OpenLevel(GetWorld(), "Town");
    }
}