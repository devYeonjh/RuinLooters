// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuinLootersGameMode.h"
#include "RuinLootersCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARuinLootersGameMode::ARuinLootersGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
