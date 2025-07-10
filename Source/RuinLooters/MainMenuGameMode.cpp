#include "MainMenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MainMenuWidget.h"

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (MainMenuWidgetClass)
    {
        UUserWidget* Menu = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
        if (Menu)
        {
            Menu->AddToViewport();

            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC)
            {
                PC->bShowMouseCursor = true;
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(Menu->TakeWidget());
                PC->SetInputMode(InputMode);
            }
        }
    }
} 