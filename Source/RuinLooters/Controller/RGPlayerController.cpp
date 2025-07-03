// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/RGPlayerController.h"

void ARGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
}
