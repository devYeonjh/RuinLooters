// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLItemBoxBase.generated.h"

UCLASS()
class RUINLOOTERS_API ARLItemBoxBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARLItemBoxBase();

protected:
	virtual void BeginPlay() override;

protected:
	// Player와 겹치는 박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UBoxComponent* DroppedItemOverlapBox;

	// 아이템 객체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UStaticMeshComponent* DroppedItemMainBody;

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName DroppedItemName;

	UPROPERTY()
	class URLGameInstance* GameInstance;

	UPROPERTY()
	class UWorld* World;


public:
	FORCEINLINE class UBoxComponent* GetDroppedItemOverlapBox() { return DroppedItemOverlapBox; };

};



