// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/RLWeapon.h"
#include "Components/BoxComponent.h"
#include "Character/RLCharacterPlayer.h"
#include "GameInstance/RLGameInstance.h"

// Sets default values
ARLWeapon::ARLWeapon()
{

}

void ARLWeapon::BeginPlay()
{
	Super::BeginPlay();

	DroppedItemOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ARLWeapon::OnPlayerOverlap);
}

void ARLWeapon::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OtherActor);
	if (Player)
	{
		DroppedWeaponRow = GameInstance->GetWeaponInformation(DroppedItemName);

		Player->ChangeWeapon(DroppedWeaponRow);

		UE_LOG(LogTemp, Warning, TEXT("Weapon Box Overlap"));

		UE_LOG(LogTemp, Warning, TEXT("AttackDamage: %d ."), Player->GetAttackDamage());

		Destroy();
	}
}




