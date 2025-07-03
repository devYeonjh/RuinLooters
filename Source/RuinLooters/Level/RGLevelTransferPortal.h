// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RGLevelTransferPortal.generated.h"

UCLASS()
class RUINLOOTERS_API ARGLevelTransferPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	ARGLevelTransferPortal();

protected:
	//  Mesh와 Material 변경 불가 설정 
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* MeshComponent;

	// 월드에 배치된 객체의 디테일 패널에서 바꾸기 위해 Custom 존재
	UPROPERTY(EditAnywhere, Category = "Visual")
	UStaticMesh* CustomMesh;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UMaterialInterface* CustomMaterial;

private:
	// 한번에 로드되는 것을 방지하기 위해 Soft로 선언
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
