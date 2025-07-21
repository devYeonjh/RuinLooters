#include "RLAIDialogueNPC.h"
#include "Character/RLCharacterPlayer.h"
#include "Controller/RLPlayerController.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ARLAIDialogueNPC::ARLAIDialogueNPC()
{
	// Create dialogue manager component
	DialogueManager = CreateDefaultSubobject<URLDialogueManager>(TEXT("DialogueManager"));
	
	// Set default values
	bUseA2FByDefault = true;
	bAutoStartDialogueOnInteract = true;
	DialogueRange = 300.0f;
	bPlayGreetingOnApproach = true;
	
	// Setup default personality
	SetupDefaultPersonality();
	
	// Set default dialogue widget class (to be set in Blueprint)
	DialogueWidgetClass = nullptr;
}

void ARLAIDialogueNPC::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize dialogue components
	InitializeDialogueComponents();
	
	UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: %s initialized"), *Personality.CharacterName);
}

void ARLAIDialogueNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// End any active dialogue
	if (IsInDialogue())
	{
		EndDialogue();
	}
	
	// Clean up UI
	if (CurrentDialogueWidget)
	{
		CurrentDialogueWidget->RemoveFromParent();
		CurrentDialogueWidget = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

bool ARLAIDialogueNPC::StartDialogue()
{
	if (!ValidateDialogueSetup())
	{
		UE_LOG(LogTemp, Error, TEXT("AIDialogueNPC: Cannot start dialogue - setup validation failed"));
		return false;
	}
	
	if (IsInDialogue())
	{
		UE_LOG(LogTemp, Warning, TEXT("AIDialogueNPC: Already in dialogue"));
		return false;
	}
	
	// Start dialogue with manager
	bool bSuccess = DialogueManager->StartConversation(this);
	if (bSuccess)
	{
		// Show dialogue widget
		ShowDialogueWidget();
		
		// Switch input mode
		SwitchToDialogueInputMode();
		
		UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Started dialogue with %s"), *Personality.CharacterName);
	}
	
	return bSuccess;
}

void ARLAIDialogueNPC::EndDialogue()
{
	if (DialogueManager)
	{
		DialogueManager->EndConversation();
	}
	
	// Hide dialogue widget
	HideDialogueWidget();
	
	// Restore game input mode
	RestoreGameInputMode();
	
	UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Ended dialogue"));
}

bool ARLAIDialogueNPC::IsInDialogue() const
{
	return DialogueManager && DialogueManager->IsInConversation();
}

void ARLAIDialogueNPC::SetPersonality(const FNPCPersonality& NewPersonality)
{
	Personality = NewPersonality;
	UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Personality updated to %s"), *Personality.CharacterName);
}

FNPCPersonality ARLAIDialogueNPC::GetPersonality() const
{
	return Personality;
}

void ARLAIDialogueNPC::ShowDialogueWidget()
{
	if (!CurrentDialogueWidget)
	{
		CreateDialogueWidget();
	}
	
	if (CurrentDialogueWidget)
	{
		CurrentDialogueWidget->SetVisibility(ESlateVisibility::Visible);
		CurrentDialogueWidget->InitializeDialogue(DialogueManager, Personality.CharacterName);
	}
}

void ARLAIDialogueNPC::HideDialogueWidget()
{
	if (CurrentDialogueWidget)
	{
		CurrentDialogueWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ARLAIDialogueNPC::PlayGreeting()
{
	if (GreetingMessages.Num() == 0)
	{
		return;
	}
	
	FString Greeting = GetRandomGreeting();
	
	// Play greeting through TTS if dialogue manager is available
	if (DialogueManager && DialogueManager->TTSManager)
	{
		DialogueManager->TTSManager->SpeakText(Greeting, Personality.SpeakerID, bUseA2FByDefault);
		UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Playing greeting: %s"), *Greeting);
	}
}

FString ARLAIDialogueNPC::GetRandomGreeting() const
{
	if (GreetingMessages.Num() == 0)
	{
		return FString::Printf(TEXT("Hello! I'm %s."), *Personality.CharacterName);
	}
	
	int32 RandomIndex = UKismetMathLibrary::RandomIntegerInRange(0, GreetingMessages.Num() - 1);
	return GreetingMessages[RandomIndex];
}

void ARLAIDialogueNPC::OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Call parent implementation first
	Super::OnDetectPlayerBoxBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	ARLCharacterPlayer* InteractCharacter = Cast<ARLCharacterPlayer>(OtherActor);
	if (InteractCharacter == Player)
	{
		// Play greeting if enabled
		if (bPlayGreetingOnApproach && !IsInDialogue())
		{
			PlayGreeting();
		}
		
		// Auto-start dialogue if enabled
		if (bAutoStartDialogueOnInteract && !IsInDialogue())
		{
			// Small delay to allow greeting to play
			FTimerHandle DelayTimer;
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [this]()
			{
				StartDialogue();
			}, 1.0f, false);
		}
		
		UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Player entered interaction range"));
	}
}

void ARLAIDialogueNPC::InitializeDialogueComponents()
{
	if (bDialogueComponentsInitialized)
	{
		return;
	}
	
	// Setup dialogue manager
	if (DialogueManager)
	{
		// Set personality in conversation context
		DialogueManager->ConversationContext.NPCPersonality = Personality;
		
		// Bind to dialogue events
		DialogueManager->OnDialogueStateChanged.AddDynamic(this, &ARLAIDialogueNPC::OnDialogueStateChanged);
		DialogueManager->OnConversationEvent.AddDynamic(this, &ARLAIDialogueNPC::OnConversationEvent);
	}
	
	// Setup default greetings if none provided
	if (GreetingMessages.Num() == 0)
	{
		GreetingMessages.Add(FString::Printf(TEXT("Hello there! I'm %s."), *Personality.CharacterName));
		GreetingMessages.Add(FString::Printf(TEXT("Greetings, traveler! My name is %s."), *Personality.CharacterName));
		GreetingMessages.Add(FString::Printf(TEXT("Welcome! I'm %s. How can I help you?"), *Personality.CharacterName));
	}
	
	bDialogueComponentsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Dialogue components initialized"));
}

void ARLAIDialogueNPC::SetupDefaultPersonality()
{
	Personality.CharacterName = TEXT("AI NPC");
	Personality.PersonalityPrompt = TEXT("You are a helpful and friendly NPC in a fantasy adventure game. You speak in a warm, welcoming manner and are always ready to help adventurers. You have knowledge about the local area and can provide guidance to travelers.");
	Personality.BackgroundStory = TEXT("I am a local resident who enjoys meeting new people and helping travelers on their journeys.");
	Personality.SpeakerID = TEXT(""); // Will use default TTS voice
	
	// Add some knowledge topics
	Personality.KnowledgeBase.Add(TEXT("Local area information"));
	Personality.KnowledgeBase.Add(TEXT("Travel advice"));
	Personality.KnowledgeBase.Add(TEXT("General game world lore"));
}

void ARLAIDialogueNPC::SetupDialogueWidget()
{
	// This will be called when creating the widget
	// Widget class should be set in Blueprint
}

void ARLAIDialogueNPC::OnDialogueStateChanged(EDialogueState NewState)
{
	UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Dialogue state changed to %d"), (int32)NewState);
	
	// Handle specific state changes if needed
	switch (NewState)
	{
	case EDialogueState::Idle:
		// Dialogue ended
		RestoreGameInputMode();
		break;
		
	case EDialogueState::WaitingForPlayer:
		// Player's turn to input
		break;
		
	case EDialogueState::Speaking:
		// NPC is speaking
		break;
		
	case EDialogueState::WaitingForLLM:
		// Waiting for AI response
		break;
	}
}

void ARLAIDialogueNPC::OnConversationEvent(const FString& EventType, const FString& EventData)
{
	UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Conversation event - %s: %s"), *EventType, *EventData);
	
	if (EventType == TEXT("ConversationEnded"))
	{
		HideDialogueWidget();
		RestoreGameInputMode();
	}
}

void ARLAIDialogueNPC::CreateDialogueWidget()
{
	if (!DialogueWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AIDialogueNPC: No DialogueWidgetClass set"));
		return;
	}
	
	if (CurrentDialogueWidget)
	{
		CurrentDialogueWidget->RemoveFromParent();
	}
	
	CurrentDialogueWidget = CreateWidget<URLDialogueWidget>(GetWorld(), DialogueWidgetClass);
	if (CurrentDialogueWidget)
	{
		CurrentDialogueWidget->AddToViewport();
		CurrentDialogueWidget->SetVisibility(ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Dialogue widget created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AIDialogueNPC: Failed to create dialogue widget"));
	}
}

void ARLAIDialogueNPC::SwitchToDialogueInputMode()
{
	if (PlayerController)
	{
		// Show mouse cursor
		PlayerController->bShowMouseCursor = true;
		
		// Set input mode to UI and game
		FInputModeGameAndUI InputMode;
		if (CurrentDialogueWidget)
		{
			InputMode.SetWidgetToFocus(CurrentDialogueWidget->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		
		// Disable player movement input mapping context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (Player && Player->GetDefaultMappingContext())
			{
				Subsystem->RemoveMappingContext(Player->GetDefaultMappingContext());
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Switched to dialogue input mode"));
	}
}

void ARLAIDialogueNPC::RestoreGameInputMode()
{
	if (PlayerController)
	{
		// Hide mouse cursor
		PlayerController->bShowMouseCursor = false;
		
		// Set input mode back to game only
		FInputModeGameOnly GameOnly;
		PlayerController->SetInputMode(GameOnly);
		
		// Re-enable player movement input mapping context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (Player && Player->GetDefaultMappingContext())
			{
				Subsystem->AddMappingContext(Player->GetDefaultMappingContext(), 0);
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("AIDialogueNPC: Restored game input mode"));
	}
}

bool ARLAIDialogueNPC::ValidateDialogueSetup() const
{
	if (!DialogueManager)
	{
		UE_LOG(LogTemp, Error, TEXT("AIDialogueNPC: DialogueManager is null"));
		return false;
	}
	
	if (!DialogueWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIDialogueNPC: DialogueWidgetClass not set - widget creation will fail"));
		return false;
	}
	
	if (Personality.CharacterName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AIDialogueNPC: Personality CharacterName is empty"));
	}
	
	return true;
}