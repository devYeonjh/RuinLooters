// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/RLPlayerController.h"

void ARLPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
}



