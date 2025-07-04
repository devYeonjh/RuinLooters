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

	// StaticMeshComponent 생성 및 세트 설정
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	MeshComponent->SetupAttachment(TransferPortal); // BoxComp의 자식으로 설정
	Wall = CreateDefaultSubobject<UBoxComponent>(TEXT("Wall"));
	Wall->SetCollisionProfileName(TEXT("BlockAll"));
	Wall->SetupAttachment(TransferPortal); // BoxComp의 자식으로 설정
	Wall->SetRelativeScale3D(FVector(4.0f, 1.0f, 6.0f));

	// 기본 설정
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
		// 현재 스테이지인지 혹은 다른 곳인지 확인
		FName GetPlayeLevelName = Player->GetLevelName();
		if (GetPlayeLevelName.ToString().Contains(TEXT("Stage")))
		{
			if (Player->GetWorldAliveEnemyCount() > 0)
			{
				// 아직 남아있는 적
				Player->SetbStageExit(true);
			}
			else
			{
				// 아직 클리어하지 않아서 못 감
				Player->SetbStageExit(false);
			}

			// 포탈에 대화창을 띄워 줄 수 있게 설정
			Player->ShowStagePortalWidget();
		}
		else
		{
			Player->SetbStageExit(false);

			// 바로 이동
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TransferLevelName);
		}
	}
}

// 에디터 상에 배치 되었을 때 호출
void ARLLevelTransferPortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// static Mesh 설정
	if (CustomMesh)
	{
		MeshComponent->SetStaticMesh(CustomMesh);
	}
	// 머티리얼 설정
	if (CustomMaterial)
	{
		MeshComponent->SetMaterial(0, CustomMaterial);
	}

}




