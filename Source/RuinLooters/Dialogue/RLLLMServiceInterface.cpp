#include "RLLLMServiceInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"

URLLMServiceInterface::URLLMServiceInterface()
{
	RequestTimeoutSeconds = 30.0f;
	bIsProcessingRequest = false;
}

void URLLMServiceInterface::BroadcastResponse(bool bSuccess, const FString& Response)
{
	// Clear timeout timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	bIsProcessingRequest = false;
	OnLLMResponse.Broadcast(bSuccess, Response);

	UE_LOG(LogTemp, Log, TEXT("LLMService: Response broadcasted - Success: %s, Response: %s"), 
		bSuccess ? TEXT("true") : TEXT("false"), *Response);
}

void URLLMServiceInterface::OnRequestTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("LLMService: Request timed out after %f seconds"), RequestTimeoutSeconds);
	BroadcastResponse(false, TEXT("Request timed out"));
}