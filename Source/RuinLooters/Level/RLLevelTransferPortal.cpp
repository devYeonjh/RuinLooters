// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/RLLevelTransferPortal.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"

// Sets default values
ARLLevelTransferPortal::ARLLevelTransferPortal()
{
	TransferPortal = CreateDefaultSubobject<UBoxComponent>(TEXT("TransferPortal"));
	RootComponent = TransferPortal;

	TransferPortal->SetRelativeScale3D(FVector(4.0f, 1.0f, 6.0f));

	// StaticMeshComponent ���� �� ��Ʈ ����
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	MeshComponent->SetupAttachment(TransferPortal); // BoxComp�� �ڽ����� ���?
	Wall = CreateDefaultSubobject<UBoxComponent>(TEXT("Wall"));
	Wall->SetCollisionProfileName(TEXT("BlockAll"));
	Wall->SetupAttachment(TransferPortal); // BoxComp�� �ڽ����� ���?	Wall->SetRelativeScale3D(FVector(4.0f, 1.0f, 6.0f));

	// �⺻ ����
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}
}

void ARLLevelTransferPortal::NotifyActorBeginOverlap(AActor* OtherAcotr)
{
	Super::NotifyActorBeginOverlap(OtherAcotr);

	ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(OtherAcotr);

	if (Player)
	{
		// ���� �����������?�� ���� ���� Ȯ��
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
void ARLLevelTransferPortal::OnConstruction(const FTransform& Transform)
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




