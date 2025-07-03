// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/RGPotion.h"
#include "GameInstance/RGGameInstance.h"
#include "Components/BoxComponent.h"
#include "Character/RGCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

ARGPotion::ARGPotion()
{
}

void ARGPotion::BeginPlay()
{
	Super::BeginPlay();

	DroppedItemOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ARGPotion::OnPlayerOverlap);
}

void ARGPotion::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(OtherActor);
	if (Player)
	{
		Potion = GameInstance->GetPotionInformation(DroppedItemName);

		Player->TakeCharacterHeal(Potion->HealAmount);

		UE_LOG(LogTemp, Warning, TEXT("HealAmount: %d ."), Potion->HealAmount);

		Destroy();
	}
}
