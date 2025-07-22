#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Base64.h"
#include "RLCosyVoiceClient.generated.h"

class URLA2FComponent;

UENUM(BlueprintType)
enum class ECosyVoiceError : uint8
{
	None,
	NetworkTimeout,
	ServerError,
	AudioProcessingFailed,
	InvalidResponse,
	A2FComponentNull,
	Base64DecodeFailed,
	SoundWaveCreationFailed
};

USTRUCT(BlueprintType)
struct FCosyVoiceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	FString ServerURL = TEXT("http://localhost:50001");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	float RequestTimeout = 30.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	FString DefaultSpeakerID = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	int32 SampleRate = 22050;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	bool bLogVerbose = false;
};

USTRUCT(BlueprintType)
struct FCosyVoiceResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	bool bSuccess = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	ECosyVoiceError ErrorCode = ECosyVoiceError::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	FString ErrorMessage;
	
	// PCM data instead of SoundWave
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	TArray<uint8> PCMData;
	
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	int32 SampleRate = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	int32 NumChannels = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	float ProcessingTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "CosyVoice")
	int32 AudioDataSize = 0;
};

// Delegate declarations (must be after struct definitions)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTTSComplete, const TArray<uint8>&, PCMData, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTTSCompleteDetailed, FCosyVoiceResult, Result);

UCLASS(BlueprintType, Blueprintable)
class RUINLOOTERS_API URLCosyVoiceClient : public UObject
{
	GENERATED_BODY()

public:
	URLCosyVoiceClient();

	// Basic TTS functionality
	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	void GenerateTTS(const FString& Text, const FString& SpeakerID = TEXT(""));
	
	// Direct A2F integration 
	UFUNCTION(BlueprintCallable, Category = "CosyVoice|A2F")
	void GenerateTTSWithA2F(const FString& Text, URLA2FComponent* A2FComponent, 
						   const FString& SpeakerID = TEXT(""));

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosyVoice")
	FCosyVoiceSettings Settings;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "CosyVoice")
	FOnTTSComplete OnTTSComplete;

	UPROPERTY(BlueprintAssignable, Category = "CosyVoice")
	FOnTTSCompleteDetailed OnTTSCompleteDetailed;

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	bool IsRequestInProgress() const { return bRequestInProgress; }

	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	void CancelCurrentRequest();

	UFUNCTION(BlueprintCallable, Category = "CosyVoice")
	void SetServerURL(const FString& NewURL) { Settings.ServerURL = NewURL; }

protected:
	virtual void BeginDestroy() override;

private:
	// HTTP processing
	void ProcessTTSRequest(const FString& Text, const FString& SpeakerID, 
						  URLA2FComponent* A2FComponent = nullptr);
	void OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, 
							  bool bSuccess, URLA2FComponent* A2FComponent, float RequestStartTime);
	
	// Request management
	TSharedPtr<IHttpRequest> CurrentRequest;
	bool bRequestInProgress = false;
	
	// Helper functions
	FString CreateRequestPayload(const FString& Text, const FString& SpeakerID);
	FCosyVoiceResult ProcessResponse(FHttpResponsePtr Response, float ProcessingTime);
	
	// Logging
	void LogVerbose(const FString& Message);
	void LogError(const FString& Message);
};

// Static utility class for audio processing
class RUINLOOTERS_API FCosyVoiceUtils
{
public:
	// Base64 decoding
	static bool DecodeBase64Audio(const FString& Base64Data, TArray<uint8>& OutPCMData);
	
	// Audio format validation
	static bool ValidateAudioFormat(int32 SampleRate, int32 NumChannels, int32 BitDepth);
	
	// Constants
	static constexpr int32 EXPECTED_SAMPLE_RATE = 22050;
	static constexpr int32 EXPECTED_CHANNELS = 1;
	static constexpr int32 EXPECTED_BIT_DEPTH = 16;
};