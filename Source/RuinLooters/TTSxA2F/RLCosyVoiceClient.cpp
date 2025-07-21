#include "RLCosyVoiceClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Misc/Base64.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

URLCosyVoiceClient::URLCosyVoiceClient()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URLCosyVoiceClient::BeginPlay()
{
	Super::BeginPlay();
	
	// Create audio importer instance
	AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
}

void URLCosyVoiceClient::GenerateTTS(const FString& Text, const FString& SpeakerID)
{
	// Store original request info for potential retry
	if (!CurrentRetryState.bIsRetrying)
	{
		CurrentRetryState.OriginalText = Text;
		CurrentRetryState.OriginalSpeaker = SpeakerID;
		CurrentRetryState.RetryCount = 0;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	// Select endpoint based on whether speaker ID is provided
	FString URL = ServerURL;
	if (SpeakerID.IsEmpty())
	{
		URL += TEXT("/tts");
	}
	else
	{
		URL += TEXT("/tts/speaker");
	}
	
	Request->SetURL(URL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	
	// Create JSON payload
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("text", Text);
	JsonObject->SetStringField("return_format", "pcm");
	JsonObject->SetNumberField("sample_rate", 22050);
	
	if (!SpeakerID.IsEmpty())
	{
		JsonObject->SetStringField("speaker_id", SpeakerID);
	}
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnTTSResponseReceived);
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("CosyVoice: TTS request sent - Text: %s, Speaker: %s"), *Text, SpeakerID.IsEmpty() ? TEXT("[none]") : *SpeakerID);
}

void URLCosyVoiceClient::GeneratePCMStreamingTTS(const FString& Text, const FString& SpeakerID, int32 SampleRate, int32 NumChannels, int32 BitsPerSample)
{
	// Store PCM parameters
	CurrentPCMParams.SampleRate = SampleRate;
	CurrentPCMParams.NumChannels = NumChannels;
	CurrentPCMParams.BitsPerSample = BitsPerSample;
	
	// Create PCM procedural sound wave
	CurrentPCMSoundWave = UPCMProceduralSoundWave::CreatePCMProceduralSoundWave(SampleRate, NumChannels);
	CurrentPCMSoundWave->ConfigureForAudioComponent();
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	// Select endpoint based on whether speaker ID is provided
	FString URL = ServerURL;
	if (SpeakerID.IsEmpty())
	{
		URL += TEXT("/tts");
	}
	else
	{
		URL += TEXT("/tts/speaker");
	}
	
	Request->SetURL(URL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	
	// Create JSON payload
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("text", Text);
	JsonObject->SetStringField("return_format", "pcm");
	JsonObject->SetNumberField("sample_rate", SampleRate);
	
	if (!SpeakerID.IsEmpty())
	{
		JsonObject->SetStringField("speaker_id", SpeakerID);
	}
	
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnPCMTTSResponseReceived);
	Request->ProcessRequest();
}

void URLCosyVoiceClient::GetSpeakerList()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL + TEXT("/speakers"));
	Request->SetVerb("GET");
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnSpeakerListResponseReceived);
	Request->ProcessRequest();
}

void URLCosyVoiceClient::CreateVoicePreset(const FString& PresetID, const FString& SampleText, const TArray<uint8>& AudioData, const FString& Description)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL + TEXT("/speakers"));
	Request->SetVerb("POST");
	
	// Create multipart form data
	FString Boundary = FGuid::NewGuid().ToString();
	Request->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	
	// Build form data
	TArray<uint8> RequestContent;
	FString FormData;
	
	// speaker_id
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"speaker_id\"\r\n\r\n");
	FormData += PresetID + TEXT("\r\n");
	
	// prompt_text
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"prompt_text\"\r\n\r\n");
	FormData += SampleText + TEXT("\r\n");
	
	// description (optional)
	if (!Description.IsEmpty())
	{
		FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
		FormData += TEXT("Content-Disposition: form-data; name=\"description\"\r\n\r\n");
		FormData += Description + TEXT("\r\n");
	}
	
	// prompt_audio
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"prompt_audio\"; filename=\"voice.wav\"\r\n");
	FormData += TEXT("Content-Type: audio/wav\r\n\r\n");
	
	// Convert text to bytes
	RequestContent.Append((uint8*)TCHAR_TO_UTF8(*FormData), FormData.Len());
	// Add audio data
	RequestContent.Append(AudioData);
	// End boundary
	FString EndBoundary = FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);
	RequestContent.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());
	
	Request->SetContent(RequestContent);
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnSpeakerOperationResponseReceived);
	Request->ProcessRequest();
}

void URLCosyVoiceClient::DeleteSpeaker(const FString& SpeakerID)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL + TEXT("/speakers/") + SpeakerID);
	Request->SetVerb("DELETE");
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnSpeakerOperationResponseReceived);
	Request->ProcessRequest();
}

void URLCosyVoiceClient::GenerateZeroShotTTS(const FString& Text, const FString& PromptText, const TArray<uint8>& PromptAudioData)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL + TEXT("/tts/zero-shot"));
	Request->SetVerb("POST");
	
	// Create multipart form data
	FString Boundary = FGuid::NewGuid().ToString();
	Request->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	
	// Build form data
	TArray<uint8> RequestContent;
	FString FormData;
	
	// text
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"text\"\r\n\r\n");
	FormData += Text + TEXT("\r\n");
	
	// prompt_text
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"prompt_text\"\r\n\r\n");
	FormData += PromptText + TEXT("\r\n");
	
	// return_format
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"return_format\"\r\n\r\n");
	FormData += TEXT("pcm\r\n");
	
	// prompt_audio
	FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
	FormData += TEXT("Content-Disposition: form-data; name=\"prompt_audio\"; filename=\"prompt.wav\"\r\n");
	FormData += TEXT("Content-Type: audio/wav\r\n\r\n");
	
	// Convert text to bytes
	RequestContent.Append((uint8*)TCHAR_TO_UTF8(*FormData), FormData.Len());
	// Add audio data
	RequestContent.Append(PromptAudioData);
	// End boundary
	FString EndBoundary = FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);
	RequestContent.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());
	
	Request->SetContent(RequestContent);
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnTTSResponseReceived);
	Request->ProcessRequest();
}

void URLCosyVoiceClient::CheckServerHealth()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL + TEXT("/health"));
	Request->SetVerb("GET");
	Request->OnProcessRequestComplete().BindUObject(this, &URLCosyVoiceClient::OnHealthCheckResponseReceived);
	Request->ProcessRequest();
}

void URLCosyVoiceClient::OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		// Success - reset retry state and process audio
		CurrentRetryState.Reset();
		
		// Parse JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString AudioDataBase64;
			int32 SampleRate = 22050;
			
			if (JsonObject->TryGetStringField(TEXT("audio_data"), AudioDataBase64))
			{
				JsonObject->TryGetNumberField(TEXT("sample_rate"), SampleRate);
				
				// Decode Base64 audio data
				TArray<uint8> AudioData = DecodeBase64(AudioDataBase64);
				
				// Process PCM audio data using RuntimeAudioImporter
				ProcessPCMAudioData(AudioData, SampleRate, 1, 16);
			}
		}
	}
	else
	{
		// Check if this is a speaker not found error and we can retry
		bool bShouldRetry = false;
		FString ResponseBody;
		
		if (Response.IsValid())
		{
			ResponseBody = Response->GetContentAsString();
			int32 ResponseCode = Response->GetResponseCode();
			
			// Check for speaker not found error (400 with specific message)
			if (ResponseCode == 400 && IsSpeakerNotFoundError(ResponseBody))
			{
				if (CurrentRetryState.RetryCount < CurrentRetryState.MaxRetries)
				{
					bShouldRetry = true;
				}
			}
		}
		
		if (bShouldRetry)
		{
			// Try fallback speaker
			FString CurrentSpeaker = CurrentRetryState.bIsRetrying ? 
				GetNextFallbackSpeaker(CurrentRetryState.OriginalSpeaker) : 
				CurrentRetryState.OriginalSpeaker;
			
			RetryTTSWithFallbackSpeaker(CurrentRetryState.OriginalText, CurrentSpeaker);
		}
		else
		{
			// Final failure - log error and reset retry state
			FString ErrorMsg = TEXT("CosyVoice TTS request failed");
			
			if (Response.IsValid())
			{
				int32 ResponseCode = Response->GetResponseCode();
				ErrorMsg += FString::Printf(TEXT(" - Status: %d, Body: %s"), ResponseCode, *ResponseBody);
			}
			else if (!bSuccess)
			{
				ErrorMsg += TEXT(" - Network connection failed");
			}
			else
			{
				ErrorMsg += TEXT(" - Invalid response");
			}
			
			if (CurrentRetryState.RetryCount > 0)
			{
				ErrorMsg += FString::Printf(TEXT(" (after %d retries)"), CurrentRetryState.RetryCount);
			}
			
			UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
			UE_LOG(LogTemp, Error, TEXT("Server URL: %s"), *ServerURL);
			
			CurrentRetryState.Reset();
			OnTTSResponse.Broadcast(false, nullptr);
		}
	}
}

void URLCosyVoiceClient::OnPCMTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		// Parse JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString AudioDataBase64;
			
			if (JsonObject->TryGetStringField(TEXT("audio_data"), AudioDataBase64))
			{
				// Decode Base64 audio data
				TArray<uint8> AudioData = DecodeBase64(AudioDataBase64);
				
				// Add PCM data to procedural sound wave
				if (CurrentPCMSoundWave)
				{
					CurrentPCMSoundWave->AddPCMData(AudioData);
					OnPCMTTSResponse.Broadcast(true, CurrentPCMSoundWave);
				}
			}
		}
	}
	else
	{
		OnPCMTTSResponse.Broadcast(false, nullptr);
		UE_LOG(LogTemp, Error, TEXT("CosyVoice PCM TTS request failed"));
	}
}

void URLCosyVoiceClient::OnSpeakerListResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	TArray<FString> SpeakerIDs;
	
	if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		// Parse JSON response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			const TSharedPtr<FJsonObject>* SpeakersObject;
			if (JsonObject->TryGetObjectField(TEXT("speakers"), SpeakersObject))
			{
				for (const auto& SpeakerPair : (*SpeakersObject)->Values)
				{
					SpeakerIDs.Add(SpeakerPair.Key);
				}
			}
		}
		
		OnSpeakerListResponse.Broadcast(true, SpeakerIDs);
	}
	else
	{
		OnSpeakerListResponse.Broadcast(false, SpeakerIDs);
		UE_LOG(LogTemp, Error, TEXT("CosyVoice speaker list request failed"));
	}
}

void URLCosyVoiceClient::OnSpeakerOperationResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	FString Message;
	
	if (bSuccess && Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();
		if (ResponseCode == 200 || ResponseCode == 201)
		{
			Message = TEXT("Operation completed successfully");
			OnSpeakerOperationResponse.Broadcast(true, Message);
		}
		else
		{
			Message = FString::Printf(TEXT("Operation failed with code: %d"), ResponseCode);
			OnSpeakerOperationResponse.Broadcast(false, Message);
		}
	}
	else
	{
		Message = TEXT("Request failed");
		OnSpeakerOperationResponse.Broadcast(false, Message);
	}
}

void URLCosyVoiceClient::OnHealthCheckResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
	if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		UE_LOG(LogTemp, Log, TEXT("✅ CosyVoice server is healthy (URL: %s)"), *ServerURL);
	}
	else
	{
		FString ErrorDetails;
		if (Response.IsValid())
		{
			ErrorDetails = FString::Printf(TEXT("Status: %d, Response: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
		}
		else
		{
			ErrorDetails = TEXT("No response - server may be offline or unreachable");
		}
		
		UE_LOG(LogTemp, Error, TEXT("❌ CosyVoice server health check failed"));
		UE_LOG(LogTemp, Error, TEXT("Server URL: %s"), *ServerURL);
		UE_LOG(LogTemp, Error, TEXT("Error Details: %s"), *ErrorDetails);
	}
}

TArray<uint8> URLCosyVoiceClient::DecodeBase64(const FString& Base64String)
{
	TArray<uint8> DecodedData;
	FBase64::Decode(Base64String, DecodedData);
	return DecodedData;
}

void URLCosyVoiceClient::ProcessPCMAudioData(const TArray<uint8>& AudioData, int32 SampleRate, int32 NumChannels, int32 BitsPerSample)
{
	if (!AudioImporter)
	{
		OnTTSResponse.Broadcast(false, nullptr);
		return;
	}
	
	// Bind to the delegate first (RuntimeAudioImporter requires this)
	AudioImporter->OnResult.AddDynamic(this, &URLCosyVoiceClient::OnAudioImportResult);
	
	// Import audio from RAW PCM buffer
	AudioImporter->ImportAudioFromRAWBuffer(AudioData, ERuntimeRAWAudioFormat::Int16, SampleRate, NumChannels);
}

void URLCosyVoiceClient::OnAudioImportResult(URuntimeAudioImporterLibrary* Importer, UImportedSoundWave* ImportedSoundWave, ERuntimeImportStatus Status)
{
	bool bSuccess = (Status == ERuntimeImportStatus::SuccessfulImport);
	
	if (bSuccess && ImportedSoundWave)
	{
		// Configure sound wave properties
		ImportedSoundWave->SetLooping(false);
		ImportedSoundWave->SetVolume(1.0f);
		
		UE_LOG(LogTemp, Log, TEXT("✅ CosyVoice: Audio imported successfully - Duration: %.2fs"), ImportedSoundWave->GetDuration());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ CosyVoice: Audio import failed - Status: %d"), (int32)Status);
	}
	
	// Broadcast the result to TTSManager
	OnTTSResponse.Broadcast(bSuccess, ImportedSoundWave);
}

void URLCosyVoiceClient::RetryTTSWithFallbackSpeaker(const FString& OriginalText, const FString& FailedSpeaker)
{
	FString NextSpeaker = GetNextFallbackSpeaker(FailedSpeaker);
	CurrentRetryState.RetryCount++;
	CurrentRetryState.bIsRetrying = true;
	
	UE_LOG(LogTemp, Warning, TEXT("CosyVoice: Speaker '%s' failed, retrying with '%s' (attempt %d/%d)"), 
		*FailedSpeaker, 
		NextSpeaker.IsEmpty() ? TEXT("[default]") : *NextSpeaker,
		CurrentRetryState.RetryCount, 
		CurrentRetryState.MaxRetries);
	
	// Retry with fallback speaker
	GenerateTTS(OriginalText, NextSpeaker);
}

FString URLCosyVoiceClient::GetNextFallbackSpeaker(const FString& CurrentSpeaker)
{
	// Fallback chain: specific_speaker -> "default" -> "" (empty/basic)
	if (CurrentSpeaker == TEXT("sample"))
	{
		return TEXT("default");
	}
	else if (CurrentSpeaker == TEXT("default"))
	{
		return TEXT(""); // Empty string for basic TTS
	}
	else if (!CurrentSpeaker.IsEmpty())
	{
		return TEXT("default"); // Try default first for any other speaker
	}
	else
	{
		return TEXT(""); // Already at basic TTS, no more fallbacks
	}
}

bool URLCosyVoiceClient::IsSpeakerNotFoundError(const FString& ErrorMessage)
{
	// Check for common speaker not found error patterns
	return ErrorMessage.Contains(TEXT("Speaker")) && 
		   (ErrorMessage.Contains(TEXT("not found")) || 
			ErrorMessage.Contains(TEXT("sample not found")) ||
			ErrorMessage.Contains(TEXT("does not exist")));
}