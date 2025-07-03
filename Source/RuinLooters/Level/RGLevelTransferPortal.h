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
	//  Mesh�� Material ���� �Ұ� ���� 
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* MeshComponent;

	// ���忡 ��ġ�� ��ü�� ������ �гο��� �ٲٱ� ���� Custom ����
	UPROPERTY(EditAnywhere, Category = "Visual")
	UStaticMesh* CustomMesh;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UMaterialInterface* CustomMaterial;

private:
	// �ѹ��� �ε�Ǵ� ���� �����ϱ� ���� Soft�� ����
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
