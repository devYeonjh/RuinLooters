// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/RGLevelTransferPortal.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RGCharacterPlayer.h"

// Sets default values
ARGLevelTransferPortal::ARGLevelTransferPortal()
{
	TransferPortal = CreateDefaultSubobject<UBoxComponent>(TEXT("TransferPortal"));
	RootComponent = TransferPortal;

	TransferPortal->SetRelativeScale3D(FVector(4.0f, 1.0f, 6.0f));

	// StaticMeshComponent ���� �� ��Ʈ ����
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	MeshComponent->SetupAttachment(TransferPortal); // BoxComp�� �ڽ����� ��

	Wall = CreateDefaultSubobject<UBoxComponent>(TEXT("Wall"));
	Wall->SetCollisionProfileName(TEXT("BlockAll"));
	Wall->SetupAttachment(TransferPortal); // BoxComp�� �ڽ����� ��
	Wall->SetRelativeScale3D(FVector(4.0f, 1.0f, 6.0f));

	// �⺻ ����
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}
}

void ARGLevelTransferPortal::NotifyActorBeginOverlap(AActor* OtherAcotr)
{
	Super::NotifyActorBeginOverlap(OtherAcotr);

	ARGCharacterPlayer* Player = Cast<ARGCharacterPlayer>(OtherAcotr);

	if (Player)
	{
		// ���� ����������� �� ���� ���� Ȯ��
		FName GetPlayeLevelName = Player->GetLevelName();
		if (GetPlayeLevelName.ToString().Contains(TEXT("Stage")))
		{
			if (Player->GetWorldAliveEnemyCount() > 0)
			{
				// ���� ������ ��
				Player->SetbStageExit(true);
			}
			else
			{
				// ���� �������� ���� �� 
				Player->SetbStageExit(false);
			}

			// ��Ż�� ������ ���� �� ���� ����
			Player->ShowStagePortalWidget();
		}
		else
		{
			Player->SetbStageExit(false);

			// ���� �̵�
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TransferLevelName);
		}
	}
}

// ���� ���� ��ġ �Ǿ��� �� ȣ��
void ARGLevelTransferPortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// static Mesh ����
	if (CustomMesh)
	{
		MeshComponent->SetStaticMesh(CustomMesh);
	}
	// ��Ƽ���� ����
	if (CustomMaterial)
	{
		MeshComponent->SetMaterial(0, CustomMaterial);
	}

}

