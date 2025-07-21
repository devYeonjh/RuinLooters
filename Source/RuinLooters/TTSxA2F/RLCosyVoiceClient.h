#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/ImportedSoundWave.h"
#include "Sound/PCMProceduralSoundWave.h"
#include "RuntimeAudioImporterLibrary.h"
#include "RLCosyVoiceClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTTSResponse, bool, bSuccess, UImportedSoundWave*, SoundWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPCMTTSResponse, bool, bSuccess, UPCMProceduralSoundWave*, ProceduralSoundWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeakerListResponse, bool, bSuccess, const TArray<FString>&, SpeakerIDs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeakerOperationResponse, bool, bSuccess, const FString&, Message);

USTRUCT(BlueprintType)
struct FCosyVoiceSpeakerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CosyVoice")
	FString SpeakerID;

	UPROPERTY(BlueprintReadWrite, Category = "CosyVoice")
	FString PromptText;

	UPROPERTY(BlueprintReadWrite, Category = "CosyVoice")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "CosyVoice")
	FDateTime CreatedAt;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLCosyVoiceClient : public UActorComponent
{
	GENERATED_BODY()

public:
	URLCosyVoiceClient();

protected:
	virtual void BeginPlay() override;

public:
	// Default server URL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice|Config")
	FString ServerURL = TEXT("http://localhost:50001");

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "CosyVoice|Delegates")
	FOnTTSResponse OnTTSResponse;

	UPROPERTY(BlueprintAssignable, Category = "CosyVoice|Delegates")
	FOnPCMTTSResponse OnPCMTTSResponse;

	UPROPERTY(BlueprintAssignable, Category = "CosyVoice|Delegates")
	FOnSpeakerListResponse OnSpeakerListResponse;

	UPROPERTY(BlueprintAssignable, Category = "CosyVoice|Delegates")
	FOnSpeakerOperationResponse OnSpeakerOperationResponse;

	// Basic TTS Functions
	UFUNCTION(BlueprintCallable, Category = "CosyVoice|TTS")
	void GenerateTTS(const FString& Text, const FString& SpeakerID = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "CosyVoice|TTS")
	void GeneratePCMStreamingTTS(const FString& Text, const FString& SpeakerID = TEXT(""), int32 SampleRate = 24000, int32 NumChannels = 1, int32 BitsPerSample = 16);

	// Speaker Management
	UFUNCTION(BlueprintCallable, Category = "CosyVoice|Speakers")
	void GetSpeakerList();

	UFUNCTION(BlueprintCallable, Category = "CosyVoice|Speakers")
	void CreateVoicePreset(const FString& PresetID, const FString& SampleText, const TArray<uint8>& AudioData, const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "CosyVoice|Speakers")
	void DeleteSpeaker(const FString& SpeakerID);

	// Zero-shot TTS (one-time without saving speaker)
	UFUNCTION(BlueprintCallable, Category = "CosyVoice|TTS")
	void GenerateZeroShotTTS(const FString& Text, const FString& PromptText, const TArray<uint8>& PromptAudioData);

	// Health check
	UFUNCTION(BlueprintCallable, Category = "CosyVoice|Utility")
	void CheckServerHealth();

private:
	// HTTP Response Handlers
	void OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void OnPCMTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void OnSpeakerListResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void OnSpeakerOperationResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void OnHealthCheckResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

	// Helper Functions
	TArray<uint8> DecodeBase64(const FString& Base64String);
	void ProcessPCMAudioData(const TArray<uint8>& AudioData, int32 SampleRate, int32 NumChannels, int32 BitsPerSample);
	
	// RuntimeAudioImporter delegate callback
	UFUNCTION()
	void OnAudioImportResult(URuntimeAudioImporterLibrary* Importer, UImportedSoundWave* ImportedSoundWave, ERuntimeImportStatus Status);

	// Speaker fallback functionality
	void RetryTTSWithFallbackSpeaker(const FString& OriginalText, const FString& FailedSpeaker);
	FString GetNextFallbackSpeaker(const FString& CurrentSpeaker);
	bool IsSpeakerNotFoundError(const FString& ErrorMessage);

	// Cached components
	UPROPERTY()
	URuntimeAudioImporterLibrary* AudioImporter;

	UPROPERTY()
	UPCMProceduralSoundWave* CurrentPCMSoundWave;

	// PCM Streaming parameters
	struct FPCMStreamingParams
	{
		int32 SampleRate;
		int32 NumChannels;
		int32 BitsPerSample;
	} CurrentPCMParams;

	// Retry state tracking
	struct FTTSRetryState
	{
		FString OriginalText;
		FString OriginalSpeaker;
		int32 RetryCount;
		int32 MaxRetries;
		bool bIsRetrying;

		FTTSRetryState()
		{
			OriginalText = TEXT("");
			OriginalSpeaker = TEXT("");
			RetryCount = 0;
			MaxRetries = 3;
			bIsRetrying = false;
		}

		void Reset()
		{
			OriginalText = TEXT("");
			OriginalSpeaker = TEXT("");
			RetryCount = 0;
			bIsRetrying = false;
		}
	} CurrentRetryState;
};