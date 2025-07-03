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

    // 게임 종료 함수(월드, 컨트롤러, 종료 방식, true면 강제종료 false면 플랫폼이 요구하는 절차대로 종료)
    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void URGPlayerDeadWidget::RestartGame()
{
    // 플레이어 체력이 0 일 때
    // 레벨 이동 => EndPlay() 실행 => 체력 0 일 때 => 초기화 및 저장
        
    if (Player)
    {
        //  타운으로 레벨 이동
        UGameplayStatics::OpenLevel(GetWorld(), "Town");
    }
}