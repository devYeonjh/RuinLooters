#include "RLDialogueManager.h"
#include "RLAIDialogueNPC.h"
#include "RLMockLLMService.h"
#include "UI/RLDialogueWidget.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

URLDialogueManager::URLDialogueManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Set default LLM service to mock for testing
	LLMServiceClass = URLMockLLMService::StaticClass();
	bUseA2FByDefault = true;
	ConversationTimeoutSeconds = 300.0f;
	MaxConversationHistory = 10;
}

void URLDialogueManager::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: No owner actor found"));
		return;
	}

	// Get or create TTS Manager
	TTSManager = Owner->FindComponentByClass<URLTTSManager>();
	if (!TTSManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager: TTSManager not found on owner, creating new one"));
		TTSManager = NewObject<URLTTSManager>(Owner);
		Owner->AddInstanceComponent(TTSManager);
		TTSManager->RegisterComponent();
	}

	// Initialize LLM service
	InitializeLLMService();
	
	// Setup event bindings
	SetupEventBindings();
	
	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialized successfully"));
}

void URLDialogueManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// End any active conversation
	if (IsInConversation())
	{
		EndConversation();
	}
	
	// Clean up event bindings
	CleanupEventBindings();
	
	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ConversationTimeoutTimer);
	}
	
	Super::EndPlay(EndPlayReason);
}

bool URLDialogueManager::StartConversation(ARLAIDialogueNPC* NPC)
{
	if (!NPC)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: Cannot start conversation with null NPC"));
		return false;
	}

	if (!CanStartNewConversation())
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot start new conversation - already in conversation or service unavailable"));
		return false;
	}

	// Set current NPC and initialize conversation
	CurrentNPC = NPC;
	ConversationContext.NPCPersonality = NPC->GetPersonality();
	ConversationContext.ConversationStartTime = FDateTime::Now();
	ConversationContext.CurrentLocation = TEXT("Game World"); // Could be enhanced with actual location
	
	// Set TTSManager target to the NPC for dynamic A2F resolution
	if (TTSManager)
	{
		TTSManager->SetTargetActor(NPC);
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: Set TTSManager target to NPC: %s"), *NPC->GetName());
	}
	
	// Clear previous conversation history
	ClearConversationHistory();
	
	// Set initial state
	SetDialogueState(EDialogueState::WaitingForPlayer);
	
	// Start conversation timeout
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ConversationTimeoutTimer,
			this,
			&URLDialogueManager::HandleConversationTimeout,
			ConversationTimeoutSeconds,
			false
		);
	}
	
	// Broadcast conversation started event
	BroadcastEvent(TEXT("ConversationStarted"), NPC->GetPersonality().CharacterName);
	
	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Started conversation with %s"), 
		*NPC->GetPersonality().CharacterName);
	
	return true;
}

void URLDialogueManager::SendPlayerMessage(const FString& Message)
{
	if (!IsInConversation() || CurrentState != EDialogueState::WaitingForPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot send message - not in valid conversation state"));
		return;
	}

	if (Message.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot send empty message"));
		return;
	}

	// Store current message
	CurrentPlayerMessage = Message;
	
	// Change state to waiting for LLM
	SetDialogueState(EDialogueState::WaitingForLLM);
	
	// Build prompt and send to LLM
	FString FullPrompt = BuildLLMPrompt(Message);
	FString Context = ConversationContext.GetFullContext();
	
	if (LLMService)
	{
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: Sending to LLM - Player: %s"), *Message);
		LLMService->GenerateResponse(FullPrompt, Context);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: LLM Service not available"));
		SetDialogueState(EDialogueState::WaitingForPlayer);
		BroadcastEvent(TEXT("Error"), TEXT("LLM Service not available"));
	}
}

void URLDialogueManager::EndConversation()
{
	if (!IsInConversation())
	{
		return;
	}

	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ConversationTimeoutTimer);
	}
	
	// Cancel any pending LLM requests
	if (LLMService && CurrentState == EDialogueState::WaitingForLLM)
	{
		LLMService->CancelRequest();
	}
	
	// Stop any ongoing TTS
	if (TTSManager && CurrentState == EDialogueState::Speaking)
	{
		TTSManager->StopCurrentTTS();
	}
	
	// Clear TTSManager target
	if (TTSManager)
	{
		TTSManager->SetTargetActor(nullptr);
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: Cleared TTSManager target"));
	}
	
	// Store final state
	FString NPCName = CurrentNPC ? CurrentNPC->GetPersonality().CharacterName : TEXT("Unknown");
	
	// Reset state
	CurrentNPC = nullptr;
	CurrentDialogueWidget = nullptr;
	CurrentPlayerMessage.Empty();
	SetDialogueState(EDialogueState::Idle);
	
	// Broadcast conversation ended event
	BroadcastEvent(TEXT("ConversationEnded"), NPCName);
	
	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Ended conversation with %s"), *NPCName);
}

bool URLDialogueManager::IsInConversation() const
{
	return CurrentState != EDialogueState::Idle && CurrentNPC != nullptr;
}

void URLDialogueManager::SetDialogueState(EDialogueState NewState)
{
	if (CurrentState != NewState)
	{
		EDialogueState OldState = CurrentState;
		CurrentState = NewState;
		
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: State changed from %d to %d"), 
			(int32)OldState, (int32)NewState);
		
		OnDialogueStateChanged.Broadcast(NewState);
	}
}

void URLDialogueManager::AddToConversationHistory(const FString& PlayerMessage, const FString& NPCResponse)
{
	FDialogueExchange Exchange;
	Exchange.PlayerMessage = PlayerMessage;
	Exchange.NPCResponse = NPCResponse;
	Exchange.Timestamp = FDateTime::Now();
	
	ConversationContext.ConversationHistory.Add(Exchange);
	
	// Trim history if it gets too long
	TrimConversationHistory();
	
	// Broadcast the exchange
	OnPlayerInput.Broadcast(PlayerMessage, NPCResponse);
}

void URLDialogueManager::ClearConversationHistory()
{
	ConversationContext.ConversationHistory.Empty();
	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Conversation history cleared"));
}

FString URLDialogueManager::BuildLLMPrompt(const FString& PlayerMessage) const
{
	FString Prompt;
	
	// Add personality and context
	Prompt += FString::Printf(TEXT("Character Name: %s\n"), *ConversationContext.NPCPersonality.CharacterName);
	Prompt += FString::Printf(TEXT("Personality: %s\n"), *ConversationContext.NPCPersonality.PersonalityPrompt);
	
	if (!ConversationContext.NPCPersonality.BackgroundStory.IsEmpty())
	{
		Prompt += FString::Printf(TEXT("Background: %s\n"), *ConversationContext.NPCPersonality.BackgroundStory);
	}
	
	// Add recent conversation history
	if (ConversationContext.ConversationHistory.Num() > 0)
	{
		Prompt += TEXT("\nRecent conversation:\n");
		int32 StartIndex = FMath::Max(0, ConversationContext.ConversationHistory.Num() - 3);
		for (int32 i = StartIndex; i < ConversationContext.ConversationHistory.Num(); i++)
		{
			const FDialogueExchange& Exchange = ConversationContext.ConversationHistory[i];
			Prompt += FString::Printf(TEXT("Player: %s\n"), *Exchange.PlayerMessage);
			Prompt += FString::Printf(TEXT("%s: %s\n"), *ConversationContext.NPCPersonality.CharacterName, *Exchange.NPCResponse);
		}
	}
	
	// Add current player message
	Prompt += FString::Printf(TEXT("\nPlayer says: %s\n"), *PlayerMessage);
	Prompt += FString::Printf(TEXT("Respond as %s:"), *ConversationContext.NPCPersonality.CharacterName);
	
	return Prompt;
}

FString URLDialogueManager::GetCurrentNPCName() const
{
	return CurrentNPC ? CurrentNPC->GetPersonality().CharacterName : TEXT("Unknown");
}

bool URLDialogueManager::CanStartNewConversation() const
{
	return CurrentState == EDialogueState::Idle && 
		   LLMService && 
		   LLMService->IsAvailable();
}

void URLDialogueManager::OnLLMResponseReceived(bool bSuccess, const FString& Response)
{
	if (CurrentState != EDialogueState::WaitingForLLM)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Received LLM response in wrong state"));
		return;
	}

	if (bSuccess && !Response.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: LLM Response: %s"), *Response);
		ProcessLLMResponse(Response);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: LLM request failed"));
		BroadcastEvent(TEXT("Error"), TEXT("Failed to get NPC response"));
		SetDialogueState(EDialogueState::WaitingForPlayer);
	}
}

void URLDialogueManager::OnTTSCompleted(bool bSuccess, const FString& Text)
{
	if (CurrentState != EDialogueState::Speaking)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("DialogueManager: TTS completed for: %s"), *Text);
	
	// Return to waiting for player input
	SetDialogueState(EDialogueState::WaitingForPlayer);
}

void URLDialogueManager::OnA2FAnimationCompleted(bool bSuccess, const FString& Text)
{
	// A2F completion is handled by TTS completion for now
	UE_LOG(LogTemp, Log, TEXT("DialogueManager: A2F animation completed for: %s"), *Text);
}

void URLDialogueManager::SetupEventBindings()
{
	if (LLMService)
	{
		LLMService->OnLLMResponse.AddDynamic(this, &URLDialogueManager::OnLLMResponseReceived);
	}
	
	if (TTSManager)
	{
		TTSManager->OnTTSCompleted.AddDynamic(this, &URLDialogueManager::OnTTSCompleted);
		TTSManager->OnTTSWithA2FCompleted.AddDynamic(this, &URLDialogueManager::OnA2FAnimationCompleted);
	}
}

void URLDialogueManager::CleanupEventBindings()
{
	if (LLMService)
	{
		LLMService->OnLLMResponse.RemoveDynamic(this, &URLDialogueManager::OnLLMResponseReceived);
	}
	
	if (TTSManager)
	{
		TTSManager->OnTTSCompleted.RemoveDynamic(this, &URLDialogueManager::OnTTSCompleted);
		TTSManager->OnTTSWithA2FCompleted.RemoveDynamic(this, &URLDialogueManager::OnA2FAnimationCompleted);
	}
}

void URLDialogueManager::InitializeLLMService()
{
	if (LLMServiceClass)
	{
		LLMService = NewObject<URLLMServiceInterface>(this, LLMServiceClass);
		if (LLMService)
		{
			LLMService->Initialize();
			UE_LOG(LogTemp, Log, TEXT("DialogueManager: LLM Service initialized: %s"), 
				*LLMService->GetServiceName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: No LLM service class specified"));
	}
}

void URLDialogueManager::ProcessLLMResponse(const FString& Response)
{
	// Add to conversation history
	AddToConversationHistory(CurrentPlayerMessage, Response);
	
	// Play NPC response
	PlayNPCResponse(Response);
}

void URLDialogueManager::PlayNPCResponse(const FString& Response)
{
	if (!TTSManager)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueManager: No TTS Manager available"));
		SetDialogueState(EDialogueState::WaitingForPlayer);
		return;
	}
	
	// Set state to speaking
	SetDialogueState(EDialogueState::Speaking);
	
	// Get speaker ID from NPC personality
	FString SpeakerID = ConversationContext.NPCPersonality.SpeakerID;
	
	// Play TTS with A2F if available
	TTSManager->SpeakText(Response, SpeakerID, bUseA2FByDefault);
	
	UE_LOG(LogTemp, Log, TEXT("DialogueManager: Playing NPC response: %s"), *Response);
}

void URLDialogueManager::HandleConversationTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Conversation timed out"));
	BroadcastEvent(TEXT("ConversationTimeout"), TEXT("Conversation ended due to timeout"));
	EndConversation();
}

void URLDialogueManager::BroadcastEvent(const FString& EventType, const FString& EventData)
{
	OnConversationEvent.Broadcast(EventType, EventData);
	UE_LOG(LogTemp, Log, TEXT("DialogueManager Event: %s - %s"), *EventType, *EventData);
}

FString URLDialogueManager::GetGreetingMessage() const
{
	// This could be enhanced to use predefined greetings from personality
	return FString::Printf(TEXT("Hello! I'm %s. How can I help you today?"), 
		*ConversationContext.NPCPersonality.CharacterName);
}

void URLDialogueManager::TrimConversationHistory()
{
	if (ConversationContext.ConversationHistory.Num() > MaxConversationHistory)
	{
		int32 ExcessCount = ConversationContext.ConversationHistory.Num() - MaxConversationHistory;
		ConversationContext.ConversationHistory.RemoveAt(0, ExcessCount);
		UE_LOG(LogTemp, Log, TEXT("DialogueManager: Trimmed %d old conversation entries"), ExcessCount);
	}
}

bool URLDialogueManager::ValidateConversationState() const
{
	return CurrentNPC != nullptr && LLMService != nullptr && TTSManager != nullptr;
}