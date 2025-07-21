#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RLDialogueTypes.generated.h"

// Dialogue state enumeration
UENUM(BlueprintType)
enum class EDialogueState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	WaitingForLLM UMETA(DisplayName = "Waiting for LLM"),
	Speaking UMETA(DisplayName = "Speaking"),
	WaitingForPlayer UMETA(DisplayName = "Waiting for Player")
};

// NPC Personality configuration
USTRUCT(BlueprintType)
struct FNPCPersonality
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	FString CharacterName = TEXT("Unnamed NPC");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	FString PersonalityPrompt = TEXT("You are a helpful NPC in a fantasy world.");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	FString BackgroundStory = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	FString SpeakerID = TEXT(""); // For TTS voice

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	TArray<FString> KnowledgeBase; // Things this NPC knows about

	FNPCPersonality()
	{
		CharacterName = TEXT("Unnamed NPC");
		PersonalityPrompt = TEXT("You are a helpful NPC in a fantasy world.");
		BackgroundStory = TEXT("");
		SpeakerID = TEXT("");
	}
};

// Single dialogue exchange between player and NPC
USTRUCT(BlueprintType)
struct FDialogueExchange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString PlayerMessage;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString NPCResponse;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FDateTime Timestamp;

	FDialogueExchange()
	{
		PlayerMessage = TEXT("");
		NPCResponse = TEXT("");
		Timestamp = FDateTime::Now();
	}
};

// Complete conversation context
USTRUCT(BlueprintType)
struct FConversationContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FNPCPersonality NPCPersonality;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TArray<FDialogueExchange> ConversationHistory;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FString CurrentLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FDateTime ConversationStartTime;

	// Helper function to build LLM prompt
	FString GetFullContext() const;

	FConversationContext()
	{
		CurrentLocation = TEXT("Unknown Location");
		ConversationStartTime = FDateTime::Now();
	}
};

// DataTable row for NPC personalities
USTRUCT(BlueprintType)
struct FNPCPersonalityTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	FNPCPersonality Personality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bUseA2FByDefault = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DefaultVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FString> DefaultGreetings;

	FNPCPersonalityTableRow()
	{
		bUseA2FByDefault = true;
		DefaultVolume = 1.0f;
		DefaultGreetings.Add(TEXT("Hello there!"));
		DefaultGreetings.Add(TEXT("Greetings, traveler!"));
	}
};

// Delegates for dialogue events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLLMResponse, bool, bSuccess, const FString&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStateChanged, EDialogueState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerInput, const FString&, PlayerMessage, const FString&, NPCResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConversationEvent, const FString&, EventType, const FString&, EventData);