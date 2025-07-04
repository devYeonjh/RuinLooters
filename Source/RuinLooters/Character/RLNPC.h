// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLNPC.generated.h"

UCLASS()
class RUINLOOTERS_API ARLNPC : public AActor
{
	GENERATED_BODY()
	
public:	
	ARLNPC();

protected:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

	// 플레이어 감지 영역
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> DetectPlayerBox;

	UPROPERTY()
	class UNPCStoreWidget* CastedStoreWiget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> StoreClass;

	UPROPERTY()
	class ARLCharacterPlayer* Player;

	UPROPERTY()
	class ARLPlayerController* PlayerController;

	UPROPERTY()
	class UWorld* World;

	UPROPERTY()
	class URLGameInstance* GameInstance;

	UPROPERTY()
	class UUserWidget* ActiveStoreWidget = nullptr;

public:
	FORCEINLINE ARLNPC* GetNPC() { return this; };
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



