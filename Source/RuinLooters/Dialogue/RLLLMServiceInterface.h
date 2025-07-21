#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RLDialogueTypes.h"
#include "RLLLMServiceInterface.generated.h"

/**
 * Abstract base class for LLM service implementations
 * This allows for different LLM providers (OpenAI, local models, etc.)
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class RUINLOOTERS_API URLLMServiceInterface : public UObject
{
	GENERATED_BODY()

public:
	URLLMServiceInterface();

	// LLM response delegate
	UPROPERTY(BlueprintAssignable, Category = "LLM Service")
	FOnLLMResponse OnLLMResponse;

	// Pure virtual functions to be implemented by concrete classes
	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	virtual void GenerateResponse(const FString& Prompt, const FString& Context) {}

	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	virtual bool IsAvailable() const { return false; }

	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	virtual void CancelRequest() {}

	// C++ implementable functions
	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	virtual void Initialize() {}

	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	virtual void Shutdown() {}

	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	virtual FString GetServiceName() const { return TEXT("Base LLM Service"); }

protected:
	// Helper function to broadcast response
	UFUNCTION(BlueprintCallable, Category = "LLM Service")
	void BroadcastResponse(bool bSuccess, const FString& Response);

	// Request timeout handling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM Service")
	float RequestTimeoutSeconds = 30.0f;

	// Current request state
	UPROPERTY(BlueprintReadOnly, Category = "LLM Service")
	bool bIsProcessingRequest = false;

private:
	FTimerHandle TimeoutTimerHandle;

	UFUNCTION()
	void OnRequestTimeout();
};