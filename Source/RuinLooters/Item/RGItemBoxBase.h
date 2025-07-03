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
	// Player�� �������� �ڽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UBoxComponent* DroppedItemOverlapBox;

	// ������ ��ü
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UStaticMeshComponent* DroppedItemMainBody;

	// ������ �̸�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName DroppedItemName;

	UPROPERTY()
	class URGGameInstance* GameInstance;

	UPROPERTY()
	class UWorld* World;


public:
	FORCEINLINE class UBoxComponent* GetDroppedItemOverlapBox() { return DroppedItemOverlapBox; };

};
