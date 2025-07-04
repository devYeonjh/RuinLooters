// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/RLItemBoxBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Character/RLCharacterBase.h"
#include "Character/RLCharacterPlayer.h"
#include "GameInstance/RLGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARLItemBoxBase::ARLItemBoxBase()
{
	DroppedItemMainBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBody"));

	SetRootComponent(DroppedItemMainBody);

	DroppedItemMainBody->SetCollisionProfileName(TEXT("NoCollision"));

	DroppedItemOverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));

	DroppedItemOverlapBox->SetupAttachment(DroppedItemMainBody);
	DroppedItemOverlapBox->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
	DroppedItemOverlapBox->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.5f));

	DroppedItemOverlapBox->SetGenerateOverlapEvents(true);

	// Query(쿼리)를 검색
	DroppedItemOverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 충돌 컴포넌트 타입
	DroppedItemOverlapBox->SetCollisionProfileName(TEXT("WeaponBox"));

}

// Called when the game starts or when spawned
void ARLItemBoxBase::BeginPlay()
{
	Super::BeginPlay();

	World = GetWorld();
	GameInstance = Cast<URLGameInstance>(UGameplayStatics::GetGameInstance(World));
}



