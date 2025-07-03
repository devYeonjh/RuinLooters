// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/RGWeapon.h"
#include "Components/BoxComponent.h"
#include "Character/RGCharacterPlayer.h"
#include "GameInstance/RGGameInstance.h"

// Sets default values
ARGWeapon::ARGWeapon()
{

}

void ARGWeapon::BeginPlay()
{
	Super::BeginPlay();

	DroppedItemOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ARGWeapon::OnPlayerOverlap);
}

void ARGWeapon::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(OtherActor);
	if (Player)
	{
		DroppedWeaponRow = GameInstance->GetWeaponInformation(DroppedItemName);

		Player->ChangeWeapon(DroppedWeaponRow);

		UE_LOG(LogTemp, Warning, TEXT("Weapon Box Overlap"));

		UE_LOG(LogTemp, Warning, TEXT("AttackDamage: %d ."), Player->GetAttackDamage());

		Destroy();
	}
}

