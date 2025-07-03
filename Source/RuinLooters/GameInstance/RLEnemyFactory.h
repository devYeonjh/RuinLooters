// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLEnemyFactory.generated.h"

UCLASS()
class RUINLOOTERS_API ARLEnemyFactory : public AActor
{
	GENERATED_BODY()
	
public:
    ARLEnemyFactory();

protected:
    // Enemy Ŭ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory")
    TSubclassOf<class ARLCharacterEnemy> EnemyClass;

    UPROPERTY()
    class URLGameInstance* GameInstance;

    UPROPERTY()
    class UWorld* World;

    UPROPERTY()
    class URLSaveGame* SaveGame;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Factory")
    void SpawnEnemy();
};



