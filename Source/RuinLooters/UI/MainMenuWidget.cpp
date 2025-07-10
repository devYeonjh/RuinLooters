#include "UI/MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LevelSequenceActor.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (StartGameButton)
        StartGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);

    if (ExitGameButton)
        ExitGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitGameClicked);
}

void UMainMenuWidget::OnStartGameClicked()
{
    RemoveFromParent(); // UI를 즉시 제거
    PlayIntroSequence();
}

void UMainMenuWidget::PlayIntroSequence()
{
    if (!IntroSequenceAsset) {
        OnSequenceFinished();
        return;
    }

    UWorld* World = GetWorld();
    if (!World) {
        OnSequenceFinished();
        return;
    }

    ALevelSequenceActor* OutActor = nullptr;
    SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
        World,
        IntroSequenceAsset,
        FMovieSceneSequencePlaybackSettings(),
        OutActor
    );

    if (SequencePlayer)
    {
        SequencePlayer->OnFinished.AddDynamic(this, &UMainMenuWidget::OnSequenceFinished);
        SequencePlayer->Play();
    }
    else
    {
        OnSequenceFinished();
    }
}

void UMainMenuWidget::OnSequenceFinished()
{
    if (!NextLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("LandscapeAutoMaterial_Island_Example"));
    }
}

void UMainMenuWidget::OnExitGameClicked()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
} 