// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RGNPC.generated.h"

UCLASS()
class RUINLOOTERS_API ARGNPC : public AActor
{
	GENERATED_BODY()
	
public:	
	ARGNPC();

protected:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

	// �÷��̾� ���� ����
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> DetectPlayerBox;

	UPROPERTY()
	class UNPCStoreWidget* CastedStoreWiget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> StoreClass;

	UPROPERTY()
	class ARGCharacterPlayer* Player;

	UPROPERTY()
	class ARGPlayerController* PlayerController;

	UPROPERTY()
	class UWorld* World;

	UPROPERTY()
	class URGGameInstance* GameInstance;

	UPROPERTY()
	class UUserWidget* ActiveStoreWidget = nullptr;

public:
	FORCEINLINE ARGNPC* GetNPC() { return this; };
	FORCEINLINE class UUserWidget* GetActiveStoreWidget() { return ActiveStoreWidget; };

protected:
	virtual void BeginPlay() override;

public:
	void RemoveWidget();

protected:
	UFUNCTION()
	virtual void OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnDetectPlayerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
