// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuinLootersGameMode.h"
#include "RuinLootersCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARuinLootersGameMode::ARuinLootersGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClassRef(TEXT("/Game/Assassin/Blueprint/BP_Player.BP_Player_C"));
	if (PlayerPawnBPClassRef.Succeeded())
	{
		DefaultPawnClass = PlayerPawnBPClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/CoreUObject.Class'/Script/Roguelike123.RLPlayerController'_C"));
	if (PlayerControllerClassRef.Succeeded())
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}
}



