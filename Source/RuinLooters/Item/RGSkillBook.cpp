// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/RGSkillBook.h"
#include "GameInstance/RGGameInstance.h"
#include "Components/BoxComponent.h"
#include "Character/RGCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

ARGSkillBook::ARGSkillBook()
{
}

void ARGSkillBook::BeginPlay()
{
	Super::BeginPlay();

	DroppedItemOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ARGSkillBook::OnPlayerOverlap);
}

void ARGSkillBook::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(OtherActor);
	if (Player)
	{
		SkillBook = GameInstance->GetSkillBookInformation(DroppedItemName);

		UE_LOG(LogTemp, Warning, TEXT("MaxHp: %d ."), Player->GetMaxHp());
		UE_LOG(LogTemp, Warning, TEXT("defence: %d ."), Player->GetDefence());

		if (SkillBook->SkillBookIndex <= 2)
		{
			Player->TakeCharacterMaxHealth(SkillBook->UpAmount);
			Player->PlayerHpChange.Broadcast(Player->GetCurrentHp(), Player->GetMaxHp());
			UE_LOG(LogTemp, Warning, TEXT("MaxHp: %d ."), Player->GetMaxHp());
		}
		else
		{
			Player->TakeCharacterDefence(SkillBook->UpAmount);
			UE_LOG(LogTemp, Warning, TEXT("defence: %d ."), Player->GetDefence());
		}

		Destroy();
	}
}

