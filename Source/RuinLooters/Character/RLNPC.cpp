// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RLNPC.h"
#include "GameInstance/RLGameInstance.h"
#include "Character/RLCharacterPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Controller/RLPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "UI/HPWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/NPCStoreWidget.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Character/RLNPCBuyInterface.h"

// Sets default values
ARLNPC::ARLNPC()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	DetectPlayerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectPlayerBox"));

	SetRootComponent(CapsuleComponent);

	Mesh->SetupAttachment(CapsuleComponent);
	DetectPlayerBox->SetupAttachment(CapsuleComponent);
	DetectPlayerBox->SetBoxExtent(FVector(120.0f, 120.0f, 80.0f));


	// 블루프린트 클래스 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> NPCWBPClass(
		TEXT("/Game/Assassin/UI/WBP_NPCStoreWidget.WBP_NPCStoreWidget_C"));
	if (NPCWBPClass.Succeeded())
	{
		StoreClass = NPCWBPClass.Class;
	}

}

void ARLNPC::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어 감지 박스 바인딩
	DetectPlayerBox->OnComponentBeginOverlap.AddDynamic(this, &ARLNPC::OnDetectPlayerBoxBeginOverlap);
	DetectPlayerBox->OnComponentEndOverlap.AddDynamic(this, &ARLNPC::OnDetectPlayerBoxEndOverlap);

	// 기본 설정 찾기
	World = GetWorld();
	GameInstance = Cast<URLGameInstance>(UGameplayStatics::GetGameInstance(World));
	Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));
	PlayerController = Cast<ARLPlayerController>(Player->GetController());

}

void ARLNPC::RemoveWidget()
{
	// NPC ���� ����� ȭ�鿡�� ����
	if (ActiveStoreWidget)
	{
		ActiveStoreWidget->SetVisibility(ESlateVisibility::Hidden);
		if (PlayerController)
		{
			PlayerController->bShowMouseCursor = false;

			FInputModeGameOnly GameOnly;
			PlayerController->SetInputMode(GameOnly);
		}
	}

	// 플레이어 입력 컨트롤 복구
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(Player->GetDefaultMappingContext(), 0);
	}
}

void ARLNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARLCharacterPlayer* InteractCharacter = Cast<ARLCharacterPlayer>(OtherActor);

	if (InteractCharacter == Player)
	{
		Player->bIsCharacterInteractWithNPC = true;

		Player->SetInteractNPC(this);

		UE_LOG(LogTemp, Warning, TEXT("Player Begin"));
	}
}

void ARLNPC::OnDetectPlayerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	RemoveWidget();

	Player->bIsCharacterInteractWithNPC = false;

	UE_LOG(LogTemp, Warning, TEXT("Player End"));
}




