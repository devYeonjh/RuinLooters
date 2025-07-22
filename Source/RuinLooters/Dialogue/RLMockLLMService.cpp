#include "RLMockLLMService.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

URLMockLLMService::URLMockLLMService()
{
	SimulatedResponseDelaySeconds = 2.0f;
	bRandomizeResponses = true;
	InitializeDefaultResponses();
}

void URLMockLLMService::Initialize()
{
	Super::Initialize();
	UE_LOG(LogTemp, Log, TEXT("MockLLMService: Initialized"));
}

void URLMockLLMService::Shutdown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResponseTimerHandle);
	}
	Super::Shutdown();
	UE_LOG(LogTemp, Log, TEXT("MockLLMService: Shutdown"));
}

FString URLMockLLMService::GetServiceName() const
{
	return TEXT("Mock LLM Service (Testing)");
}

void URLMockLLMService::GenerateResponse(const FString& Prompt, const FString& Context)
{
	if (bIsProcessingRequest)
	{
		UE_LOG(LogTemp, Warning, TEXT("MockLLMService: Already processing a request"));
		return;
	}

	bIsProcessingRequest = true;
	UE_LOG(LogTemp, Log, TEXT("MockLLMService: Generating response for prompt: %s"), *Prompt);

	// Extract player input from prompt (simple parsing)
	FString PlayerInput = Prompt;
	if (Prompt.Contains(TEXT("Player says:")))
	{
		FString LeftPart, RightPart;
		if (Prompt.Split(TEXT("Player says:"), &LeftPart, &RightPart))
		{
			PlayerInput = RightPart.TrimStartAndEnd();
		}
	}

	// Extract NPC name from context
	FString NPCName = TEXT("NPC");
	if (Context.Contains(TEXT("Character Name:")))
	{
		FString LeftPart, RightPart;
		if (Context.Split(TEXT("Character Name:"), &LeftPart, &RightPart))
		{
			FString TempName = RightPart.TrimStartAndEnd();
			if (TempName.Split(TEXT("\n"), &NPCName, &RightPart))
			{
				NPCName = NPCName.TrimStartAndEnd();
			}
		}
	}

	// Generate mock response
	PendingResponse = GenerateMockResponse(PlayerInput, NPCName);

	// Simulate processing delay
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ResponseTimerHandle,
			this,
			&URLMockLLMService::DeliverPendingResponse,
			SimulatedResponseDelaySeconds,
			false
		);
	}
	else
	{
		// Fallback: deliver immediately if no world context
		DeliverPendingResponse();
	}
}

bool URLMockLLMService::IsAvailable() const
{
	return true; // Mock service is always available
}

void URLMockLLMService::CancelRequest()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResponseTimerHandle);
	}
	
	if (bIsProcessingRequest)
	{
		bIsProcessingRequest = false;
		UE_LOG(LogTemp, Log, TEXT("MockLLMService: Request cancelled"));
		BroadcastResponse(false, TEXT("Request was cancelled"));
	}
}

FString URLMockLLMService::GenerateMockResponse(const FString& PlayerInput, const FString& NPCName)
{
	// Simple keyword-based responses for testing
	FString LowerInput = PlayerInput.ToLower();
	
	if (LowerInput.Contains(TEXT("hello")) || LowerInput.Contains(TEXT("hi")) || LowerInput.Contains(TEXT("greet")))
	{
		return FString::Printf(TEXT("Hello there! I'm %s. Nice to meet you!"), *NPCName);
	}
	else if (LowerInput.Contains(TEXT("how are you")) || LowerInput.Contains(TEXT("how do you do")))
	{
		return FString::Printf(TEXT("I'm doing well, thank you for asking! How are you doing today?"));
	}
	else if (LowerInput.Contains(TEXT("what")) && LowerInput.Contains(TEXT("name")))
	{
		return FString::Printf(TEXT("My name is %s. What's yours?"), *NPCName);
	}
	else if (LowerInput.Contains(TEXT("bye")) || LowerInput.Contains(TEXT("goodbye")) || LowerInput.Contains(TEXT("farewell")))
	{
		return FString::Printf(TEXT("Farewell, traveler! Safe travels!"));
	}
	else if (LowerInput.Contains(TEXT("help")))
	{
		return FString::Printf(TEXT("I'd be happy to help! What do you need assistance with?"));
	}
	else if (LowerInput.Contains(TEXT("weather")))
	{
		return FString::Printf(TEXT("The weather has been quite nice lately, don't you think?"));
	}
	else if (LowerInput.Contains(TEXT("quest")) || LowerInput.Contains(TEXT("adventure")))
	{
		return FString::Printf(TEXT("Ah, an adventurer! I've heard there are interesting challenges in the area."));
	}
	else
	{
		// Fallback to random responses or contextual ones
		if (bRandomizeResponses && DefaultResponses.Num() > 0)
		{
			return GetRandomResponse();
		}
		else
		{
			return FString::Printf(TEXT("That's interesting! Tell me more about that."));
		}
	}
}

FString URLMockLLMService::GetRandomResponse()
{
	if (DefaultResponses.Num() == 0)
	{
		return TEXT("I see. That's quite interesting!");
	}

	int32 RandomIndex = UKismetMathLibrary::RandomIntegerInRange(0, DefaultResponses.Num() - 1);
	return DefaultResponses[RandomIndex];
}

void URLMockLLMService::DeliverPendingResponse()
{
	UE_LOG(LogTemp, Log, TEXT("MockLLMService: Delivering response: %s"), *PendingResponse);
	BroadcastResponse(true, PendingResponse);
	PendingResponse.Empty();
}

void URLMockLLMService::InitializeDefaultResponses()
{
	DefaultResponses.Empty();
	DefaultResponses.Add(TEXT("That's fascinating! I've never thought about it that way."));
	DefaultResponses.Add(TEXT("Interesting perspective! What made you think of that?"));
	DefaultResponses.Add(TEXT("I see what you mean. Can you elaborate on that?"));
	DefaultResponses.Add(TEXT("That reminds me of something I heard once..."));
	DefaultResponses.Add(TEXT("You raise a good point! I'll have to consider that."));
	DefaultResponses.Add(TEXT("I appreciate you sharing that with me."));
	DefaultResponses.Add(TEXT("That sounds like quite an experience!"));
	DefaultResponses.Add(TEXT("I can understand why you'd feel that way."));
	DefaultResponses.Add(TEXT("You seem to know a lot about this topic!"));
	DefaultResponses.Add(TEXT("That's a creative way to look at things."));
}