#include "RLDialogueWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

URLDialogueWidget::URLDialogueWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerMessageColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f); // Light blue
	NPCMessageColor = FLinearColor(0.3f, 1.0f, 0.3f, 1.0f);    // Light green
	SystemMessageColor = FLinearColor(1.0f, 1.0f, 0.3f, 1.0f); // Light yellow
	MaxDisplayedMessages = 20;
}

void URLDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup button event bindings
	if (SendButton)
	{
		SendButton->OnClicked.AddDynamic(this, &URLDialogueWidget::OnSendButtonClicked);
	}
	
	if (EndConversationButton)
	{
		EndConversationButton->OnClicked.AddDynamic(this, &URLDialogueWidget::OnEndConversationButtonClicked);
	}
	
	if (MessageInputBox)
	{
		MessageInputBox->OnTextCommitted.AddDynamic(this, &URLDialogueWidget::OnMessageInputCommitted);
	}
	
	// Initialize UI state
	SetInteractionEnabled(false);
	UpdateStatusText(TEXT("Ready to start conversation"));
	
	UE_LOG(LogTemp, Log, TEXT("DialogueWidget: UI constructed"));
}

void URLDialogueWidget::NativeDestruct()
{
	CleanupEventBindings();
	Super::NativeDestruct();
}

void URLDialogueWidget::InitializeDialogue(URLDialogueManager* InDialogueManager, const FString& NPCName)
{
	if (!InDialogueManager)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueWidget: Cannot initialize with null DialogueManager"));
		return;
	}
	
	DialogueManager = InDialogueManager;
	bIsInitialized = true;
	
	// Set NPC name
	if (NPCNameText)
	{
		NPCNameText->SetText(FText::FromString(FString::Printf(TEXT("Talking to: %s"), *NPCName)));
	}
	
	// Clear previous conversation
	ClearConversationDisplay();
	
	// Setup event bindings
	SetupEventBindings();
	
	// Enable interaction
	SetInteractionEnabled(true);
	UpdateStatusText(TEXT("Type your message and press Enter or click Send"));
	
	// Focus input box
	FocusInputBox();
	
	// Add welcome message
	AddSystemMessage(FString::Printf(TEXT("Conversation started with %s"), *NPCName));
	
	UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Initialized for NPC: %s"), *NPCName);
}

void URLDialogueWidget::SendMessage()
{
	if (!bIsInitialized || !DialogueManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: Cannot send message - not initialized"));
		return;
	}
	
	FString MessageText = GetCurrentInputText();
	if (MessageText.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: Cannot send empty message"));
		return;
	}
	
	// Add player message to display
	AddMessageToHistory(TEXT("You"), MessageText, true);
	
	// Send to dialogue manager
	DialogueManager->SendPlayerMessage(MessageText);
	
	// Clear input
	ClearInputText();
	
	// Update status
	UpdateStatusText(TEXT("Waiting for response..."));
	SetInteractionEnabled(false);
	
	UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Sent message: %s"), *MessageText);
}

void URLDialogueWidget::EndConversation()
{
	if (DialogueManager)
	{
		DialogueManager->EndConversation();
	}
	
	AddSystemMessage(TEXT("Conversation ended"));
	SetInteractionEnabled(false);
	UpdateStatusText(TEXT("Conversation ended"));
	
	UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Conversation ended"));
}

void URLDialogueWidget::AddMessageToHistory(const FString& Speaker, const FString& Message, bool bIsPlayerMessage)
{
	if (!ConversationHistory)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: ConversationHistory widget not found"));
		return;
	}
	
	// Choose color based on speaker
	FLinearColor MessageColor = bIsPlayerMessage ? PlayerMessageColor : NPCMessageColor;
	
	// Create speaker label
	FString DisplayText = FString::Printf(TEXT("%s: %s"), *Speaker, *Message);
	UTextBlock* MessageBlock = CreateMessageTextBlock(DisplayText, MessageColor);
	
	if (MessageBlock)
	{
		ConversationHistory->AddChild(MessageBlock);
		CurrentMessageCount++;
		
		// Trim old messages if needed
		TrimMessageHistory();
		
		// Scroll to bottom
		ScrollToBottom();
	}
}

void URLDialogueWidget::AddSystemMessage(const FString& Message)
{
	if (!ConversationHistory)
	{
		return;
	}
	
	FString DisplayText = FString::Printf(TEXT("*** %s ***"), *Message);
	UTextBlock* MessageBlock = CreateMessageTextBlock(DisplayText, SystemMessageColor);
	
	if (MessageBlock)
	{
		ConversationHistory->AddChild(MessageBlock);
		CurrentMessageCount++;
		TrimMessageHistory();
		ScrollToBottom();
	}
}

void URLDialogueWidget::UpdateStatusText(const FString& Status)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Status));
	}
}

void URLDialogueWidget::SetInteractionEnabled(bool bEnabled)
{
	if (MessageInputBox)
	{
		MessageInputBox->SetIsEnabled(bEnabled);
	}
	
	if (SendButton)
	{
		SendButton->SetIsEnabled(bEnabled);
	}
}

void URLDialogueWidget::ClearConversationDisplay()
{
	if (ConversationHistory)
	{
		ConversationHistory->ClearChildren();
		CurrentMessageCount = 0;
	}
}

void URLDialogueWidget::OnSendButtonClicked()
{
	SendMessage();
}

void URLDialogueWidget::OnEndConversationButtonClicked()
{
	EndConversation();
}

void URLDialogueWidget::OnMessageInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SendMessage();
	}
}

void URLDialogueWidget::OnDialogueStateChanged(EDialogueState NewState)
{
	switch (NewState)
	{
	case EDialogueState::Idle:
		UpdateStatusText(TEXT("Conversation ended"));
		SetInteractionEnabled(false);
		break;
		
	case EDialogueState::WaitingForLLM:
		UpdateStatusText(TEXT("Thinking..."));
		SetInteractionEnabled(false);
		break;
		
	case EDialogueState::Speaking:
		UpdateStatusText(TEXT("Speaking..."));
		SetInteractionEnabled(false);
		break;
		
	case EDialogueState::WaitingForPlayer:
		UpdateStatusText(TEXT("Your turn to speak"));
		SetInteractionEnabled(true);
		FocusInputBox();
		break;
	}
}

void URLDialogueWidget::OnPlayerInput(const FString& PlayerMessage, const FString& NPCResponse)
{
	// Player message was already added when sent, just add NPC response
	if (DialogueManager)
	{
		FString NPCName = DialogueManager->GetCurrentNPCName();
		AddMessageToHistory(NPCName, NPCResponse, false);
	}
}

void URLDialogueWidget::OnConversationEvent(const FString& EventType, const FString& EventData)
{
	if (EventType == TEXT("ConversationStarted"))
	{
		AddSystemMessage(FString::Printf(TEXT("Started conversation with %s"), *EventData));
	}
	else if (EventType == TEXT("ConversationEnded"))
	{
		AddSystemMessage(TEXT("Conversation ended"));
		SetInteractionEnabled(false);
	}
	else if (EventType == TEXT("Error"))
	{
		AddSystemMessage(FString::Printf(TEXT("Error: %s"), *EventData));
		SetInteractionEnabled(true);
	}
	else if (EventType == TEXT("ConversationTimeout"))
	{
		AddSystemMessage(TEXT("Conversation timed out"));
		SetInteractionEnabled(false);
	}
}

void URLDialogueWidget::SetupEventBindings()
{
	if (DialogueManager)
	{
		DialogueManager->OnDialogueStateChanged.AddDynamic(this, &URLDialogueWidget::OnDialogueStateChanged);
		DialogueManager->OnPlayerInput.AddDynamic(this, &URLDialogueWidget::OnPlayerInput);
		DialogueManager->OnConversationEvent.AddDynamic(this, &URLDialogueWidget::OnConversationEvent);
	}
}

void URLDialogueWidget::CleanupEventBindings()
{
	if (DialogueManager)
	{
		DialogueManager->OnDialogueStateChanged.RemoveDynamic(this, &URLDialogueWidget::OnDialogueStateChanged);
		DialogueManager->OnPlayerInput.RemoveDynamic(this, &URLDialogueWidget::OnPlayerInput);
		DialogueManager->OnConversationEvent.RemoveDynamic(this, &URLDialogueWidget::OnConversationEvent);
	}
}

void URLDialogueWidget::ScrollToBottom()
{
	if (ConversationScrollBox)
	{
		ConversationScrollBox->ScrollToEnd();
	}
}

UTextBlock* URLDialogueWidget::CreateMessageTextBlock(const FString& Text, const FLinearColor& Color)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>();
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(Text));
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetAutoWrapText(true);
		
		// Set font and styling
		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = 14;
		TextBlock->SetFont(FontInfo);
	}
	
	return TextBlock;
}

void URLDialogueWidget::TrimMessageHistory()
{
	if (!ConversationHistory || CurrentMessageCount <= MaxDisplayedMessages)
	{
		return;
	}
	
	// Remove oldest messages
	int32 MessagesToRemove = CurrentMessageCount - MaxDisplayedMessages;
	for (int32 i = 0; i < MessagesToRemove; i++)
	{
		if (ConversationHistory->GetChildrenCount() > 0)
		{
			ConversationHistory->RemoveChildAt(0);
			CurrentMessageCount--;
		}
	}
}

FString URLDialogueWidget::GetCurrentInputText() const
{
	if (MessageInputBox)
	{
		return MessageInputBox->GetText().ToString();
	}
	return TEXT("");
}

void URLDialogueWidget::ClearInputText()
{
	if (MessageInputBox)
	{
		MessageInputBox->SetText(FText::GetEmpty());
	}
}

void URLDialogueWidget::FocusInputBox()
{
	if (MessageInputBox)
	{
		MessageInputBox->SetFocus();
	}
}