#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UOpenAIClient.generated.h"

// Forward declarations for Unreal HTTP types
class IHttpRequest;
class IHttpResponse;
typedef TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FHttpRequestPtr;
typedef TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FHttpResponsePtr;

DECLARE_DELEGATE_OneParam(FT2TResponseDelegate, const FString&);

UCLASS(BlueprintType)
class RUINLOOTERS_API UOpenAIClient : public UObject
{
    GENERATED_BODY()
public:
    UOpenAIClient();

    // Delegate for T2T response
    FT2TResponseDelegate OnT2TResponseReceived;

    // Send prompt to Gemini (Google API)
    UFUNCTION(BlueprintCallable, Category="OpenAI")
    void SendPromptToGPT(const FString& Prompt);

private:
    // Correct signature for HTTP response handler
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
}; 