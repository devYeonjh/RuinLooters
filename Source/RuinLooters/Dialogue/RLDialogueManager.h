#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RLDialogueTypes.h"
#include "RLLLMServiceInterface.h"
#include "TTSxA2F/RLTTSManager.h"
#include "RLDialogueManager.generated.h"

// Forward declarations
class ARLAIDialogueNPC;
class URLDialogueWidget;

/**
 * Core component that orchestrates AI NPC dialogue
 * Manages conversation flow, LLM integration, and TTS/A2F output
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLDialogueManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URLDialogueManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Current dialogue state
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	EDialogueState CurrentState = EDialogueState::Idle;

	// Conversation context
	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FConversationContext ConversationContext;

	// Component references
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue|Components")
	URLLMServiceInterface* LLMService;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue|Components")
	URLTTSManager* TTSManager;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Config")
	TSubclassOf<URLLMServiceInterface> LLMServiceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Config")
	bool bUseA2FByDefault = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Config")
	float ConversationTimeoutSeconds = 300.0f; // 5 minutes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Config")
	int32 MaxConversationHistory = 10;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueStateChanged OnDialogueStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnPlayerInput OnPlayerInput;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnConversationEvent OnConversationEvent;

	// Main dialogue functions
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartConversation(ARLAIDialogueNPC* NPC);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SendPlayerMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndConversation();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsInConversation() const;

	// State management
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetDialogueState(EDialogueState NewState);

	// Conversation management
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AddToConversationHistory(const FString& PlayerMessage, const FString& NPCResponse);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearConversationHistory();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString BuildLLMPrompt(const FString& PlayerMessage) const;

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GetCurrentNPCName() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool CanStartNewConversation() const;

private:
	// Current conversation state
	UPROPERTY()
	ARLAIDialogueNPC* CurrentNPC = nullptr;

	UPROPERTY()
	URLDialogueWidget* CurrentDialogueWidget = nullptr;

	// Timers
	FTimerHandle ConversationTimeoutTimer;

	// Current message being processed
	FString CurrentPlayerMessage;

	// Event handlers
	UFUNCTION()
	void OnLLMResponseReceived(bool bSuccess, const FString& Response);

	UFUNCTION()
	void OnTTSCompleted(bool bSuccess, const FString& Text);

	UFUNCTION()
	void OnA2FAnimationCompleted(bool bSuccess, const FString& Text);

	// Internal functions
	void SetupEventBindings();
	void CleanupEventBindings();
	void InitializeLLMService();
	void ProcessLLMResponse(const FString& Response);
	void PlayNPCResponse(const FString& Response);
	void HandleConversationTimeout();
	void BroadcastEvent(const FString& EventType, const FString& EventData);

	// Helper functions
	FString GetGreetingMessage() const;
	void TrimConversationHistory();
	bool ValidateConversationState() const;
};