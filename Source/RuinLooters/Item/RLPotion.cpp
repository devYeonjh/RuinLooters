// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/RLPotion.h"
#include "GameInstance/RLGameInstance.h"
#include "Components/BoxComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

ARLPotion::ARLPotion()
{
}

void ARLPotion::BeginPlay()
{
	Super::BeginPlay();

	DroppedItemOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ARLPotion::OnPlayerOverlap);

	Potion = GameInstance->GetPotionInformation(DroppedItemName);

	DroppedItemMainBody->SetStaticMesh(Potion->StaticMesh);
}

void ARLPotion::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OtherActor);
	if (Player)
	{
		Player->TakeCharacterHeal(Potion->HealAmount);

		UE_LOG(LogTemp, Warning, TEXT("HealAmount: %d ."), Potion->HealAmount);

		Destroy();
	}
}



