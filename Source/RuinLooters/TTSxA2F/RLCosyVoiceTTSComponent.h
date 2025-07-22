#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RLCosyVoiceClient.h"
#include "RLCosyVoiceTTSComponent.generated.h"

class URLA2FComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLCosyVoiceTTSComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URLCosyVoiceTTSComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Main TTS functionality
	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	void SpeakText(const FString& Text, const FString& SpeakerID = TEXT(""));

	// TTS with automatic A2F integration
	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	void SpeakTextWithFacialAnimation(const FString& Text, const FString& SpeakerID = TEXT(""));

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	FCosyVoiceSettings VoiceSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	bool bAutoFindA2FComponent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice", meta = (EditCondition = "!bAutoFindA2FComponent"))
	TSoftObjectPtr<URLA2FComponent> ManualA2FComponent;

	// Events - Forward from voice client
	UPROPERTY(BlueprintAssignable, Category = "CosyVoice")
	FOnTTSComplete OnTTSComplete;

	UPROPERTY(BlueprintAssignable, Category = "CosyVoice")
	FOnTTSCompleteDetailed OnTTSCompleteDetailed;

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	bool IsRequestInProgress() const;

	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	void CancelCurrentRequest();

	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	URLA2FComponent* GetA2FComponent();

private:
	UPROPERTY()
	URLCosyVoiceClient* VoiceClient;

	UPROPERTY()
	URLA2FComponent* CachedA2FComponent;

	// Event handlers
	UFUNCTION()
	void OnVoiceClientTTSComplete(const TArray<uint8>& PCMData, bool bSuccess);

	UFUNCTION()
	void OnVoiceClientTTSCompleteDetailed(FCosyVoiceResult Result);

	// Helper functions
	void InitializeVoiceClient();
	URLA2FComponent* FindA2FComponent();
};