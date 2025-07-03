// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RGItemBoxBase.generated.h"

UCLASS()
class RUINLOOTERS_API ARGItemBoxBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARGItemBoxBase();

protected:
	virtual void BeginPlay() override;

protected:
	// Player와 오버랩할 박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UBoxComponent* DroppedItemOverlapBox;

	// 아이템 형체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UStaticMeshComponent* DroppedItemMainBody;

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName DroppedItemName;

	UPROPERTY()
	class URGGameInstance* GameInstance;

	UPROPERTY()
	class UWorld* World;


public:
	FORCEINLINE class UBoxComponent* GetDroppedItemOverlapBox() { return DroppedItemOverlapBox; };

};
