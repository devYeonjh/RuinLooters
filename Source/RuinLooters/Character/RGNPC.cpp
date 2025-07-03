// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RGNPC.h"
#include "GameInstance/RGGameInstance.h"
#include "Character/RGCharacterPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Controller/RGPlayerController.h"
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
#include "Character/RGNPCBuyInterface.h"

// Sets default values
ARGNPC::ARGNPC()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	DetectPlayerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectPlayerBox"));

	SetRootComponent(CapsuleComponent);

	Mesh->SetupAttachment(CapsuleComponent);
	DetectPlayerBox->SetupAttachment(CapsuleComponent);
	DetectPlayerBox->SetBoxExtent(FVector(120.0f, 120.0f, 80.0f));


	// ��������Ʈ Ŭ���� ����
	static ConstructorHelpers::FClassFinder<UUserWidget> NPCWBPClass(
		TEXT("/Game/Assassin/UI/WBP_NPCStoreWidget.WBP_NPCStoreWidget_C"));
	if (NPCWBPClass.Succeeded())
	{
		StoreClass = NPCWBPClass.Class;
	}

}

void ARGNPC::BeginPlay()
{
	Super::BeginPlay();

	// �÷��̾� ���� �ڽ� ���ε�
	DetectPlayerBox->OnComponentBeginOverlap.AddDynamic(this, &ARGNPC::OnDetectPlayerBoxBeginOverlap);
	DetectPlayerBox->OnComponentEndOverlap.AddDynamic(this, &ARGNPC::OnDetectPlayerBoxEndOverlap);

	// �⺻ ��� ã��
	World = GetWorld();
	GameInstance = Cast<URGGameInstance>(UGameplayStatics::GetGameInstance(World));
	Player = Cast<ARGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));
	PlayerController = Cast<ARGPlayerController>(Player->GetController());

}

void ARGNPC::RemoveWidget()
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

	// �÷��̾� ��Ʈ�� �Է� ��ǽ�
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(Player->GetDefaultMappingContext(), 0);
	}
}

void ARGNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARGCharacterPlayer* InteractCharacter = Cast<ARGCharacterPlayer>(OtherActor);

	if (InteractCharacter == Player)
	{
		Player->bIsCharacterInteractWithNPC = true;

		Player->SetInteractNPC(this);

		UE_LOG(LogTemp, Warning, TEXT("Player Begin"));
	}
}

void ARGNPC::OnDetectPlayerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	RemoveWidget();

	Player->bIsCharacterInteractWithNPC = false;

	UE_LOG(LogTemp, Warning, TEXT("Player End"));
}

