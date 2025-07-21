#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Dialogue/RLDialogueTypes.h"
#include "Dialogue/RLDialogueManager.h"
#include "RLDialogueWidget.generated.h"

/**
 * UI Widget for AI NPC dialogue system
 * Provides player input and conversation display
 */
UCLASS()
class RUINLOOTERS_API URLDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URLDialogueWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI Components (to be bound in Blueprint)
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* MessageInputBox;

	UPROPERTY(meta = (BindWidget))
	UButton* SendButton;

	UPROPERTY(meta = (BindWidget))
	UButton* EndConversationButton;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ConversationScrollBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NPCNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ConversationHistory;

public:
	// Dialogue manager reference
	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	URLDialogueManager* DialogueManager;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Widget")
	FLinearColor PlayerMessageColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Widget")
	FLinearColor NPCMessageColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Widget")
	FLinearColor SystemMessageColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Widget")
	int32 MaxDisplayedMessages = 20;

	// Main functions
	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void InitializeDialogue(URLDialogueManager* InDialogueManager, const FString& NPCName);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void SendMessage();

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void EndConversation();

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void AddMessageToHistory(const FString& Speaker, const FString& Message, bool bIsPlayerMessage = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void AddSystemMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void UpdateStatusText(const FString& Status);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Dialogue Widget")
	void ClearConversationDisplay();

private:
	// Event handlers
	UFUNCTION()
	void OnSendButtonClicked();

	UFUNCTION()
	void OnEndConversationButtonClicked();

	UFUNCTION()
	void OnMessageInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnDialogueStateChanged(EDialogueState NewState);

	UFUNCTION()
	void OnPlayerInput(const FString& PlayerMessage, const FString& NPCResponse);

	UFUNCTION()
	void OnConversationEvent(const FString& EventType, const FString& EventData);

	// Helper functions
	void SetupEventBindings();
	void CleanupEventBindings();
	void ScrollToBottom();
	UTextBlock* CreateMessageTextBlock(const FString& Text, const FLinearColor& Color);
	void TrimMessageHistory();
	FString GetCurrentInputText() const;
	void ClearInputText();
	void FocusInputBox();

	// State tracking
	bool bIsInitialized = false;
	int32 CurrentMessageCount = 0;
};