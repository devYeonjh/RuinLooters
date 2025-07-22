#pragma once

#include "CoreMinimal.h"
#include "Character/RLNPC.h"
#include "RLDialogueTypes.h"
#include "RLDialogueManager.h"
#include "UI/RLDialogueWidget.h"
#include "RLAIDialogueNPC.generated.h"

/**
 * AI-powered dialogue NPC that extends the base RLNPC
 * Supports intelligent conversations using LLM integration
 */
UCLASS(BlueprintType, Blueprintable)
class RUINLOOTERS_API ARLAIDialogueNPC : public ARLNPC
{
	GENERATED_BODY()

public:
	ARLAIDialogueNPC();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// NPC Personality configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue")
	FNPCPersonality Personality;

	// Dialogue system components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Dialogue")
	URLDialogueManager* DialogueManager;

	// UI Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue|UI")
	TSubclassOf<URLDialogueWidget> DialogueWidgetClass;
	// Dialogue settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue|Settings")
	bool bUseA2FByDefault = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue|Settings")
	bool bAutoStartDialogueOnInteract = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue|Settings")
	float DialogueRange = 300.0f;

	// Greeting configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue|Greetings")
	TArray<FString> GreetingMessages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Dialogue|Greetings")
	bool bPlayGreetingOnApproach = true;

	// Main dialogue functions
	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	bool StartDialogue();

	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	bool IsInDialogue() const;

	// Personality management
	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	void SetPersonality(const FNPCPersonality& NewPersonality);

	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	FNPCPersonality GetPersonality() const;

	// UI management
	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	void ShowDialogueWidget();

	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	void HideDialogueWidget();

	// Greeting system
	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	void PlayGreeting();

	UFUNCTION(BlueprintCallable, Category = "AI Dialogue")
	FString GetRandomGreeting() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Dialogue")
	UChildActorComponent* MetaHumanComponent;
	
	
protected:
	// Override base NPC interaction
	virtual void OnDetectPlayerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	// Initialize components and settings
	void InitializeDialogueComponents();
	void SetupDefaultPersonality();
	void SetupDialogueWidget();

	// Event handlers
	UFUNCTION()
	void OnDialogueStateChanged(EDialogueState NewState);

	UFUNCTION()
	void OnConversationEvent(const FString& EventType, const FString& EventData);

private:
	// UI state
	UPROPERTY()
	URLDialogueWidget* CurrentDialogueWidget = nullptr;

	// Component setup flags
	bool bDialogueComponentsInitialized = false;

	// Helper functions
	void CreateDialogueWidget();
	void SwitchToDialogueInputMode();
	void RestoreGameInputMode();
	bool ValidateDialogueSetup() const;
};

