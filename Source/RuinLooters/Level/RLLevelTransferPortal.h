// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLLevelTransferPortal.generated.h"

UCLASS()
class RUINLOOTERS_API ARLLevelTransferPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	ARLLevelTransferPortal();

protected:
	//  Mesh와 Material 등의 기본 설정 
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* MeshComponent;

	// 레벨에 배치한 객체의 모양을 다르게 바꾸기 위한 Custom 설정
	UPROPERTY(EditAnywhere, Category = "Visual")
	UStaticMesh* CustomMesh;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UMaterialInterface* CustomMaterial;

private:
	// 한번에 로딩되지 않게 하기 위해 Soft로 설정
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> TransferLevelName;

	UPROPERTY()
	class UBoxComponent* TransferPortal;

	UPROPERTY()
	class UBoxComponent* Wall;

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherAcotr) override;

	virtual void OnConstruction(const FTransform& Transform) override;


};



