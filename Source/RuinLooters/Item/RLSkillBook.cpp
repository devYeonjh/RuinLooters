// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/RLSkillBook.h"
#include "GameInstance/RLGameInstance.h"
#include "Components/BoxComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "Kismet/GameplayStatics.h"

ARLSkillBook::ARLSkillBook()
{
}

void ARLSkillBook::BeginPlay()
{
	Super::BeginPlay();

	DroppedItemOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ARLSkillBook::OnPlayerOverlap);
}

void ARLSkillBook::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OtherActor);
	if (Player)
	{
		SkillBook = GameInstance->GetSkillBookInformation(DroppedItemName);

		UE_LOG(LogTemp, Warning, TEXT("MaxHp: %.1f ."), Player->GetMaxHp());
		UE_LOG(LogTemp, Warning, TEXT("defence: %d ."), Player->GetDefence());

		if (SkillBook->SkillBookIndex <= 2)
		{
			Player->TakeCharacterMaxHealth(SkillBook->UpAmount);
			Player->PlayerHpChange.Broadcast(Player->GetCurrentHp(), Player->GetMaxHp());
			UE_LOG(LogTemp, Warning, TEXT("MaxHp: %.1f ."), Player->GetMaxHp());
		}
		else
		{
			Player->TakeCharacterDefence(SkillBook->UpAmount);
			UE_LOG(LogTemp, Warning, TEXT("defence: %d ."), Player->GetDefence());
		}

		Destroy();
	}
}




