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

	// StaticMeshComponent 생성 및 루트 설정
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	MeshComponent->SetupAttachment(TransferPortal); // BoxComp에 자식으로 들어감

	Wall = CreateDefaultSubobject<UBoxComponent>(TEXT("Wall"));
	Wall->SetCollisionProfileName(TEXT("BlockAll"));
	Wall->SetupAttachment(TransferPortal); // BoxComp에 자식으로 들어감
	Wall->SetRelativeScale3D(FVector(4.0f, 1.0f, 6.0f));

	// 기본 세팅
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
		// 만약 스테이지라면 적 존재 여부 확인
		FName GetPlayeLevelName = Player->GetLevelName();
		if (GetPlayeLevelName.ToString().Contains(TEXT("Stage")))
		{
			if (Player->GetWorldAliveEnemyCount() > 0)
			{
				// 적이 존재할 때
				Player->SetbStageExit(true);
			}
			else
			{
				// 적이 존재하지 않을 때 
				Player->SetbStageExit(false);
			}

			// 포탈에 오버랩 했을 때 위젯 띄우기
			Player->ShowStagePortalWidget();
		}
		else
		{
			Player->SetbStageExit(false);

			// 레벨 이동
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), TransferLevelName);
		}
	}
}

// 월드 내에 배치 되었을 때 호출
void ARGLevelTransferPortal::OnConstruction(const FTransform& Transform)
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

