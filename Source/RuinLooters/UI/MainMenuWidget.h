#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "MainMenuWidget.generated.h"
UCLASS()
class RUINLOOTERS_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnStartGameClicked();

    void PlayIntroSequence();

    UFUNCTION()
    void OnSequenceFinished();

    UFUNCTION()
    void OnExitGameClicked();

public:
    UPROPERTY(meta = (BindWidget))
    class UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* ExitGameButton;

    // 시퀀스 에셋을 에디터에서 할당
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    ULevelSequence* IntroSequenceAsset;

    // 시퀀스 플레이어를 저장
    UPROPERTY()
    ULevelSequencePlayer* SequencePlayer;

    // 시퀀스 끝나면 이동할 레벨 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FName NextLevelName = "LandscapeAutoMaterial_Island_Example";
}; 