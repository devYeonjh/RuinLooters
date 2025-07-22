#include "RLCosyVoiceClient.h"
#include "RLA2FComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "Async/Async.h"

DEFINE_LOG_CATEGORY_STATIC(LogRLCosyVoice, Log, All);

URLCosyVoiceClient::URLCosyVoiceClient()
{
	bRequestInProgress = false;
	CurrentRequest = nullptr;
}

void URLCosyVoiceClient::BeginDestroy()
{
	if (bRequestInProgress && CurrentRequest.IsValid())
	{
		LogVerbose(TEXT("Canceling request during object destruction"));
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
		bRequestInProgress = false;
	}
	
	Super::BeginDestroy();
}

void URLCosyVoiceClient::GenerateTTS(const FString& Text, const FString& SpeakerID)
{
	if (Text.IsEmpty() || Text.TrimStartAndEnd().IsEmpty())
	{
		LogError(TEXT("GenerateTTS: Text is empty"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = TEXT("Input text is empty");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	if (Text.Len() > 1000)
	{
		LogError(FString::Printf(TEXT("GenerateTTS: Text too long (%d chars, max 1000)"), Text.Len()));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = FString::Printf(TEXT("Text too long (%d characters, max 1000)"), Text.Len());
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	ProcessTTSRequest(Text, SpeakerID);
}

void URLCosyVoiceClient::GenerateTTSWithA2F(const FString& Text, URLA2FComponent* A2FComponent, const FString& SpeakerID)
{
	if (!A2FComponent)
	{
		LogError(TEXT("GenerateTTSWithA2F: A2FComponent is null"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::A2FComponentNull;
		Result.ErrorMessage = TEXT("A2FComponent is null");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	if (Text.IsEmpty() || Text.TrimStartAndEnd().IsEmpty())
	{
		LogError(TEXT("GenerateTTSWithA2F: Text is empty"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = TEXT("Input text is empty");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	if (Text.Len() > 1000)
	{
		LogError(FString::Printf(TEXT("GenerateTTSWithA2F: Text too long (%d chars, max 1000)"), Text.Len()));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = FString::Printf(TEXT("Text too long (%d characters, max 1000)"), Text.Len());
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	ProcessTTSRequest(Text, SpeakerID, A2FComponent);
}

void URLCosyVoiceClient::CancelCurrentRequest()
{
	if (bRequestInProgress && CurrentRequest.IsValid())
	{
		LogVerbose(TEXT("Canceling current TTS request"));
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
		bRequestInProgress = false;
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::NetworkTimeout;
		Result.ErrorMessage = TEXT("Request cancelled by user");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
	}
}

void URLCosyVoiceClient::ProcessTTSRequest(const FString& Text, const FString& SpeakerID, URLA2FComponent* A2FComponent)
{
	if (bRequestInProgress)
	{
		LogError(TEXT("ProcessTTSRequest: Request already in progress"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = TEXT("Request already in progress");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	if (!FHttpModule::Get().IsHttpEnabled())
	{
		LogError(TEXT("ProcessTTSRequest: HTTP module is not enabled"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::ServerError;
		Result.ErrorMessage = TEXT("HTTP module is not enabled");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	// Create HTTP request
	CurrentRequest = FHttpModule::Get().CreateRequest();
	if (!CurrentRequest.IsValid())
	{
		LogError(TEXT("ProcessTTSRequest: Failed to create HTTP request"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::ServerError;
		Result.ErrorMessage = TEXT("Failed to create HTTP request");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	// Configure request
	const FString Endpoint = SpeakerID.IsEmpty() ? TEXT("/tts") : TEXT("/tts/speaker");
	const FString FullURL = Settings.ServerURL + Endpoint;
	
	CurrentRequest->SetURL(FullURL);
	CurrentRequest->SetVerb(TEXT("POST"));
	CurrentRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	CurrentRequest->SetHeader(TEXT("User-Agent"), TEXT("RuinLooters-CosyVoiceClient/1.0"));
	CurrentRequest->SetTimeout(Settings.RequestTimeout);
	
	// Create JSON payload
	const FString RequestPayload = CreateRequestPayload(Text, SpeakerID);
	CurrentRequest->SetContentAsString(RequestPayload);
	
	LogVerbose(FString::Printf(TEXT("Sending TTS request to: %s"), *FullURL));
	LogVerbose(FString::Printf(TEXT("Request payload: %s"), *RequestPayload));
	
	// Set callback
	const float RequestStartTime = FPlatformTime::Seconds();
	CurrentRequest->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnTTSResponseReceived, A2FComponent, RequestStartTime);
	
	// Send request
	bRequestInProgress = true;
	if (!CurrentRequest->ProcessRequest())
	{
		LogError(TEXT("ProcessTTSRequest: Failed to process HTTP request"));
		
		bRequestInProgress = false;
		CurrentRequest.Reset();
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::NetworkTimeout;
		Result.ErrorMessage = TEXT("Failed to process HTTP request");
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
	}
}

void URLCosyVoiceClient::OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, 
											  bool bSuccess, URLA2FComponent* A2FComponent, float RequestStartTime)
{
	// Reset request state
	bRequestInProgress = false;
	CurrentRequest.Reset();
	
	const float ProcessingTime = FPlatformTime::Seconds() - RequestStartTime;
	
	if (!bSuccess || !Response.IsValid())
	{
		LogError(TEXT("OnTTSResponseReceived: HTTP request failed"));
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::NetworkTimeout;
		Result.ErrorMessage = TEXT("HTTP request failed");
		Result.ProcessingTime = ProcessingTime;
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	const int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode != 200)
	{
		// Get detailed error message from server response
		FString ServerResponse = Response->GetContentAsString();
		LogError(FString::Printf(TEXT("OnTTSResponseReceived: Server returned error code %d"), ResponseCode));
		LogError(FString::Printf(TEXT("Server response: %s"), *ServerResponse));
		
		// Try to parse JSON error message
		FString DetailedError = FString::Printf(TEXT("Server error: %d"), ResponseCode);
		TSharedPtr<FJsonObject> JsonResponse;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ServerResponse);
		
		if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
		{
			FString Detail;
			if (JsonResponse->TryGetStringField(TEXT("detail"), Detail))
			{
				DetailedError = FString::Printf(TEXT("Server error (%d): %s"), ResponseCode, *Detail);
			}
		}
		
		FCosyVoiceResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = (ResponseCode == 400) ? ECosyVoiceError::InvalidResponse : ECosyVoiceError::ServerError;
		Result.ErrorMessage = DetailedError;
		Result.ProcessingTime = ProcessingTime;
		
		OnTTSCompleteDetailed.Broadcast(Result);
		OnTTSComplete.Broadcast(TArray<uint8>(), false);
		return;
	}
	
	// Process response
	FCosyVoiceResult Result = ProcessResponse(Response, ProcessingTime);
	
	// Integrate with A2F if component provided using direct PCM processing
	if (Result.bSuccess && A2FComponent && Result.PCMData.Num() > 0)
	{
		LogVerbose(TEXT("Integrating with A2F component using direct PCM processing"));
		bool bA2FSuccess = A2FComponent->ExecuteA2FAnimationFromPCMDataSync(
			Result.PCMData, 
			Result.SampleRate, 
			Result.NumChannels
		);
		
		if (!bA2FSuccess)
		{
			LogError(TEXT("A2F integration failed"));
		}
	}
	
	// Broadcast results
	OnTTSCompleteDetailed.Broadcast(Result);
	OnTTSComplete.Broadcast(Result.PCMData, Result.bSuccess);
}

FString URLCosyVoiceClient::CreateRequestPayload(const FString& Text, const FString& SpeakerID)
{
	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	
	JsonPayload->SetStringField(TEXT("text"), Text);
	JsonPayload->SetStringField(TEXT("return_format"), TEXT("pcm"));
	JsonPayload->SetNumberField(TEXT("sample_rate"), Settings.SampleRate);
	
	if (!SpeakerID.IsEmpty())
	{
		JsonPayload->SetStringField(TEXT("speaker_id"), SpeakerID);
	}
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);
	
	return OutputString;
}

FCosyVoiceResult URLCosyVoiceClient::ProcessResponse(FHttpResponsePtr Response, float ProcessingTime)
{
	FCosyVoiceResult Result;
	Result.ProcessingTime = ProcessingTime;
	
	// Parse JSON response
	TSharedPtr<FJsonObject> JsonResponse;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	
	if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
	{
		LogError(TEXT("ProcessResponse: Failed to parse JSON response"));
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = TEXT("Failed to parse JSON response");
		return Result;
	}
	
	// Extract audio data
	FString AudioDataBase64;
	if (!JsonResponse->TryGetStringField(TEXT("audio_data"), AudioDataBase64))
	{
		LogError(TEXT("ProcessResponse: Missing audio_data field in response"));
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::InvalidResponse;
		Result.ErrorMessage = TEXT("Missing audio_data field in response");
		return Result;
	}
	
	int32 SampleRate;
	if (!JsonResponse->TryGetNumberField(TEXT("sample_rate"), SampleRate))
	{
		LogError(TEXT("ProcessResponse: Missing sample_rate field in response"));
		SampleRate = Settings.SampleRate; // Use default
	}
	
	// Decode Base64 audio
	TArray<uint8> PCMData;
	if (!FCosyVoiceUtils::DecodeBase64Audio(AudioDataBase64, PCMData))
	{
		LogError(TEXT("ProcessResponse: Failed to decode Base64 audio data"));
		Result.bSuccess = false;
		Result.ErrorCode = ECosyVoiceError::Base64DecodeFailed;
		Result.ErrorMessage = TEXT("Failed to decode Base64 audio data");
		return Result;
	}
	
	// Store PCM data directly in result (no SoundWave creation)
	Result.bSuccess = true;
	Result.ErrorCode = ECosyVoiceError::None;
	Result.PCMData = PCMData;
	Result.SampleRate = SampleRate;
	const int32 NumChannels = 1; // CosyVoice outputs mono audio by default
	Result.NumChannels = NumChannels;
	Result.AudioDataSize = PCMData.Num();
	
	// Calculate audio duration for logging
	float AudioDuration = 0.0f;
	if (SampleRate > 0 && NumChannels > 0)
	{
		int32 NumSamples = PCMData.Num() / (2 * NumChannels); // 16-bit = 2 bytes per sample
		AudioDuration = static_cast<float>(NumSamples) / SampleRate;
	}
	
	LogVerbose(FString::Printf(TEXT("Successfully generated TTS audio: %d bytes, %.2fs duration (%d Hz, %d channels)"), 
							  PCMData.Num(), AudioDuration, SampleRate, NumChannels));
	
	return Result;
}

void URLCosyVoiceClient::LogVerbose(const FString& Message)
{
	if (Settings.bLogVerbose)
	{
		UE_LOG(LogRLCosyVoice, Verbose, TEXT("%s"), *Message);
	}
}

void URLCosyVoiceClient::LogError(const FString& Message)
{
	UE_LOG(LogRLCosyVoice, Error, TEXT("%s"), *Message);
}

// Static utility functions implementation
bool FCosyVoiceUtils::DecodeBase64Audio(const FString& Base64Data, TArray<uint8>& OutPCMData)
{
	if (Base64Data.IsEmpty())
	{
		UE_LOG(LogRLCosyVoice, Error, TEXT("DecodeBase64Audio: Base64 data is empty"));
		return false;
	}
	
	if (!FBase64::Decode(Base64Data, OutPCMData))
	{
		UE_LOG(LogRLCosyVoice, Error, TEXT("DecodeBase64Audio: Failed to decode Base64 data"));
		return false;
	}
	
	if (OutPCMData.Num() == 0)
	{
		UE_LOG(LogRLCosyVoice, Error, TEXT("DecodeBase64Audio: Decoded data is empty"));
		return false;
	}
	
	return true;
}


bool FCosyVoiceUtils::ValidateAudioFormat(int32 SampleRate, int32 NumChannels, int32 BitDepth)
{
	if (SampleRate <= 0 || SampleRate > 96000)
	{
		UE_LOG(LogRLCosyVoice, Error, TEXT("ValidateAudioFormat: Invalid sample rate %d"), SampleRate);
		return false;
	}
	
	if (NumChannels <= 0 || NumChannels > 8)
	{
		UE_LOG(LogRLCosyVoice, Error, TEXT("ValidateAudioFormat: Invalid channel count %d"), NumChannels);
		return false;
	}
	
	if (BitDepth != 8 && BitDepth != 16 && BitDepth != 24 && BitDepth != 32)
	{
		UE_LOG(LogRLCosyVoice, Error, TEXT("ValidateAudioFormat: Invalid bit depth %d"), BitDepth);
		return false;
	}
	
	return true;
}