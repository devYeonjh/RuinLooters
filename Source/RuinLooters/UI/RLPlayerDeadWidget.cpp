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

    // 현재 월드 함수(월드, 컨트롤러, 종료 타입 true는 백그라운드 false는 플레이어가 요구하는 프로그램 종료)
    UKismetSystemLibrary::QuitGame(CurrentWorld, CurrentWorld->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void URLPlayerDeadWidget::RestartGame()
{
    // 플레이어 체력을 0 로 두기
    // 레벨 이동 => EndPlay() 호출 => 체력 0 로 죽 => 초기화 후 재생
        
    if (Player)
    {
        //  타운으로 레벨 이동
        UGameplayStatics::OpenLevel(GetWorld(), "Town");
    }
}


