// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RGEnemyFactory.generated.h"

UCLASS()
class RUINLOOTERS_API ARGEnemyFactory : public AActor
{
	GENERATED_BODY()
	
public:
    ARGEnemyFactory();

protected:
    // Enemy Ŭ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory")
    TSubclassOf<class ARGCharacterEnemy> EnemyClass;

    UPROPERTY()
    class URGGameInstance* GameInstance;

    UPROPERTY()
    class UWorld* World;

    UPROPERTY()
    class URGSaveGame* SaveGame;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Factory")
    void SpawnEnemy();
};
