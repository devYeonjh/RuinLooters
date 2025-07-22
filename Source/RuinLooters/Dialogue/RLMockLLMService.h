#pragma once

#include "CoreMinimal.h"
#include "RLLLMServiceInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "RLMockLLMService.generated.h"

/**
 * Mock LLM service for testing purposes
 * Returns predefined responses with simulated delay
 */
UCLASS(BlueprintType, Blueprintable)
class RUINLOOTERS_API URLMockLLMService : public URLLMServiceInterface
{
	GENERATED_BODY()

public:
	URLMockLLMService();

	// Override virtual functions
	virtual void Initialize() override;
	virtual void Shutdown() override;
	virtual FString GetServiceName() const override;

	// Override virtual functions from base class (no UFUNCTION needed - inherited from base)
	virtual void GenerateResponse(const FString& Prompt, const FString& Context) override;
	virtual bool IsAvailable() const override;
	virtual void CancelRequest() override;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mock LLM")
	float SimulatedResponseDelaySeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mock LLM")
	TArray<FString> DefaultResponses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mock LLM")
	bool bRandomizeResponses = true;

private:
	// Response generation
	FString GenerateMockResponse(const FString& PlayerInput, const FString& NPCName);
	FString GetRandomResponse();
	
	// Timer for simulated delay
	FTimerHandle ResponseTimerHandle;
	FString PendingResponse;

	UFUNCTION()
	void DeliverPendingResponse();

	// Response templates
	void InitializeDefaultResponses();
};