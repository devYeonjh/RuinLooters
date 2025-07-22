#include "RLA2FComponent.h"
#include "ACEBlueprintLibrary.h"
#include "ACERuntimeModule.h"
#include "ACEAudioCurveSourceComponent.h"
#include "AnimDataConsumer.h"
#include "Engine/World.h"

URLA2FComponent::URLA2FComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentAsyncAction = nullptr;
	bIsInitialized = false;
}

void URLA2FComponent::BeginPlay()
{
	Super::BeginPlay();
	// Remove automatic initialization - use lazy loading instead
	// InitializeDefaultParameters();
}

void URLA2FComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up A2F resources if initialized
	if (bIsInitialized)
	{
		UACEBlueprintLibrary::FreeA2F3DResources(ProviderName);
		UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: Freed A2F resources"));
	}
	
	Super::EndPlay(EndPlayReason);
}

void URLA2FComponent::ExecuteA2FAnimation(const FString& WavFilePath)
{
	if (WavFilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: WavFilePath is empty"));
		return;
	}

	// Ensure A2F is initialized before use
	EnsureInitialized();

	// Configure provider settings if using remote provider
	ConfigureProviderSettings();

	if (CurrentAsyncAction)
	{
		CurrentAsyncAction->AudioSendCompleted.RemoveDynamic(this, &URLA2FComponent::OnAnimationCompleted);
		CurrentAsyncAction = nullptr;
	}

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	FName CurrentProvider = GetCurrentProviderName();
	CurrentAsyncAction = UAsyncActionAnimateCharacter::AnimateCharacterFromWavFileAsync(
		GetWorld(),
		GetOwner(),
		WavFilePath,
		EmotionParams,
		FaceParams,
		CurrentProvider
	);

	if (CurrentAsyncAction)
	{
		CurrentAsyncAction->AudioSendCompleted.AddDynamic(this, &URLA2FComponent::OnAnimationCompleted);
		CurrentAsyncAction->Activate();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RLA2FComponent: Failed to create async action"));
	}
}

void URLA2FComponent::ExecuteA2FAnimationFromSoundWave(USoundWave* SoundWave)
{
	if (!SoundWave)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: SoundWave is null"));
		return;
	}

	// Ensure A2F is initialized before use
	EnsureInitialized();

	// Configure provider settings if using remote provider
	ConfigureProviderSettings();

	if (CurrentAsyncAction)
	{
		CurrentAsyncAction->AudioSendCompleted.RemoveDynamic(this, &URLA2FComponent::OnAnimationCompleted);
		CurrentAsyncAction = nullptr;
	}

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	FName CurrentProvider = GetCurrentProviderName();
	CurrentAsyncAction = UAsyncActionAnimateCharacter::AnimateCharacterFromSoundWaveAsync(
		GetWorld(),
		GetOwner(),
		SoundWave,
		EmotionParams,
		FaceParams,
		CurrentProvider
	);

	if (CurrentAsyncAction)
	{
		CurrentAsyncAction->AudioSendCompleted.AddDynamic(this, &URLA2FComponent::OnAnimationCompleted);
		CurrentAsyncAction->Activate();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RLA2FComponent: Failed to create async action from SoundWave"));
	}
}

bool URLA2FComponent::ExecuteA2FAnimationFromSoundWaveSync(USoundWave* SoundWave)
{
	if (!SoundWave)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: SoundWave is null"));
		return false;
	}

	// Ensure A2F is initialized before use
	EnsureInitialized();

	// Configure provider settings if using remote provider
	ConfigureProviderSettings();

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	FName CurrentProvider = GetCurrentProviderName();
	// Use synchronous version from UACEBlueprintLibrary
	bool bSuccess = UACEBlueprintLibrary::AnimateCharacterFromSoundWave(
		GetOwner(),
		SoundWave,
		EmotionParams,
		FaceParams,
		CurrentProvider
	);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: Sync animation failed"));
	}

	return bSuccess;
}

bool URLA2FComponent::ExecuteA2FAnimationFromPCMDataSync(const TArray<uint8>& PCMData, int32 SampleRate, int32 NumChannels)
{
	if (PCMData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: PCM data is empty"));
		return false;
	}

	// Ensure A2F is initialized before use
	EnsureInitialized();

	// Configure provider settings if using remote provider
	ConfigureProviderSettings();

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	// Convert PCM bytes to int16 samples
	const int32 NumSamples = PCMData.Num() / 2; // 16-bit samples = 2 bytes per sample
	TArray<int16> SamplesInt16;
	SamplesInt16.Reserve(NumSamples);

	// Convert bytes to int16 with little-endian interpretation (CosyVoice format)
	for (int32 i = 0; i < NumSamples; ++i)
	{
		int32 ByteIndex = i * 2;
		if (ByteIndex + 1 < PCMData.Num())
		{
			uint8 LowByte = PCMData[ByteIndex];
			uint8 HighByte = PCMData[ByteIndex + 1];
			int16 Sample = static_cast<int16>((HighByte << 8) | LowByte);
			SamplesInt16.Add(Sample);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: Processing %d PCM samples (%d Hz, %d channels) directly to A2F"), 
		   SamplesInt16.Num(), SampleRate, NumChannels);

	// Find ACEAudioCurveSourceComponent that implements IACEAnimDataConsumer interface
	UACEAudioCurveSourceComponent* ACEComp = GetOwner()->GetComponentByClass<UACEAudioCurveSourceComponent>();
	if (!ACEComp)
	{
		UE_LOG(LogTemp, Error, TEXT("RLA2FComponent: No UACEAudioCurveSourceComponent found on actor %s"), *GetOwner()->GetName());
		return false;
	}

	FName CurrentProvider = GetCurrentProviderName();
	// Use ACE runtime module to send samples directly to A2F
	// This bypasses USoundWave completely, avoiding the SoundWaveProxy assertion
	bool bSuccess = FACERuntimeModule::Get().AnimateFromAudioSamples(
		ACEComp, // IACEAnimDataConsumer* Consumer
		TArrayView<const int16>(SamplesInt16), // PCM samples
		NumChannels, // Number of channels
		SampleRate, // Sample rate
		true, // bEndOfSamples - this is a complete audio clip
		EmotionParams, // Emotion parameters
		FaceParams, // Audio2Face parameters
		CurrentProvider // A2F provider name
	);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: Direct PCM to A2F animation failed"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: Direct PCM to A2F animation succeeded"));
	}

	return bSuccess;
}

void URLA2FComponent::OnAnimationCompleted(bool bSuccess)
{
	CurrentAsyncAction = nullptr;
	
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: Animation completed with failure"));
	}
}

void URLA2FComponent::InitializeDefaultParameters()
{
	EmotionParams.OverallEmotionStrength = 0.6f;
	EmotionParams.DetectedEmotionContrast = 1.0f;
	EmotionParams.MaxDetectedEmotions = 3;
	EmotionParams.DetectedEmotionSmoothing = 0.7f;
	EmotionParams.bEnableEmotionOverride = false;
	EmotionParams.EmotionOverrideStrength = 0.5f;

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	// Debug: Log available providers (only when actually initialized)
	TArray<FName> AvailableProviders = UACEBlueprintLibrary::GetAvailableA2FProviderNames();
	UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: Lazy Loading - Available A2F Providers:"));
	for (const FName& Provider : AvailableProviders)
	{
		UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Provider.ToString());
	}
}

void URLA2FComponent::EnsureInitialized()
{
	if (bIsInitialized)
	{
		return;
	}

	FScopeLock Lock(&InitializationCS);
	
	// Double-check pattern
	if (bIsInitialized)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: Performing lazy initialization..."));
	
	InitializeDefaultParameters();
	
	// Force setup local provider to avoid RPC connection issues
	SetupLocalProvider(LocalModelType);
	
	// Pre-allocate A2F resources for optimal performance
	FName CurrentProvider = GetCurrentProviderName();
	UACEBlueprintLibrary::AllocateA2F3DResources(CurrentProvider);
	
	bIsInitialized = true;
	
	UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: Lazy initialization completed"));
}

bool URLA2FComponent::SetupAPIProvider(const FString& ServerURL, const FString& APIKey, const FString& FunctionId, const FString& FunctionVersion)
{
	if (ServerURL.IsEmpty() || APIKey.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: ServerURL or APIKey is empty"));
		return false;
	}

	// Validate API key format
	if (!APIKey.StartsWith(TEXT("nvapi-")))
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: API Key should start with 'nvapi-'"));
		return false;
	}

	// Update component settings
	bUseRemoteProvider = true;
	RemoteServerURL = ServerURL;
	NVIDIAAPIKey = APIKey;
	NvCFFunctionId = FunctionId;
	NvCFFunctionVersion = FunctionVersion;

	// Configure ACE connection info
	FACEConnectionInfo ConnectionInfo;
	ConnectionInfo.DestURL = ServerURL;
	ConnectionInfo.APIKey = APIKey;
	ConnectionInfo.NvCFFunctionId = FunctionId;
	ConnectionInfo.NvCFFunctionVersion = FunctionVersion;

	// Set connection info for the remote provider
	UACEBlueprintLibrary::SetA2XConnectionInfo(ConnectionInfo, RemoteProviderName);

	UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: API Provider configured - URL: %s, Provider: %s"), 
		   *ServerURL, *RemoteProviderName.ToString());

	return true;
}

bool URLA2FComponent::SetupLocalProvider(EA2FLocalModel ModelType)
{
	// Update component settings
	bUseRemoteProvider = false;
	LocalModelType = ModelType;

	// Get the appropriate provider name for the model type
	FName CurrentLocalProviderName = GetLocalProviderName(ModelType);

	// Configure ACE connection info for local provider
	FACEConnectionInfo ConnectionInfo;
	// Local providers typically don't need a URL or API key
	ConnectionInfo.DestURL = "";
	ConnectionInfo.APIKey = "";

	// Set connection info for the specific local provider
	UACEBlueprintLibrary::SetA2XConnectionInfo(ConnectionInfo, CurrentLocalProviderName);

	UE_LOG(LogTemp, Log, TEXT("RLA2FComponent: Local Provider configured - Model: %s, Provider: %s"), 
		   ModelType == EA2FLocalModel::Mark ? TEXT("Mark") :
		   ModelType == EA2FLocalModel::Claire ? TEXT("Claire") : TEXT("James"),
		   *CurrentLocalProviderName.ToString());

	return true;
}

TArray<FString> URLA2FComponent::GetAvailableProviders() const
{
	TArray<FName> ProviderNames = UACEBlueprintLibrary::GetAvailableA2FProviderNames();
	TArray<FString> ProviderStrings;
	
	for (const FName& CurrentProviderName : ProviderNames)
	{
		ProviderStrings.Add(CurrentProviderName.ToString());
	}
	
	return ProviderStrings;
}

bool URLA2FComponent::IsLocalProviderAvailable(EA2FLocalModel ModelType) const
{
	// Get the provider name for the specified model type
	FName RequiredProviderName = GetLocalProviderName(ModelType);
	
	// Get list of available providers
	TArray<FName> AvailableProviders = UACEBlueprintLibrary::GetAvailableA2FProviderNames();
	
	// Check if the required provider is available
	return AvailableProviders.Contains(RequiredProviderName);
}

FName URLA2FComponent::GetLocalProviderName(EA2FLocalModel ModelType) const
{
	switch (ModelType)
	{
		case EA2FLocalModel::Mark:
			return LocalProviderName_Mark;
		case EA2FLocalModel::Claire:
			return LocalProviderName_Claire;
		case EA2FLocalModel::James:
			return LocalProviderName_James;
		default:
			return LocalProviderName_Mark; // Default to Mark
	}
}

void URLA2FComponent::ConfigureProviderSettings()
{
	if (bUseRemoteProvider)
	{
		// Configure remote provider connection
		if (!NVIDIAAPIKey.IsEmpty() && !RemoteServerURL.IsEmpty())
		{
			FACEConnectionInfo ConnectionInfo;
			ConnectionInfo.DestURL = RemoteServerURL;
			ConnectionInfo.APIKey = NVIDIAAPIKey;
			ConnectionInfo.NvCFFunctionId = NvCFFunctionId;
			ConnectionInfo.NvCFFunctionVersion = NvCFFunctionVersion;

			UACEBlueprintLibrary::SetA2XConnectionInfo(ConnectionInfo, RemoteProviderName);
		}
	}
	else
	{
		// Configure local provider connection
		FACEConnectionInfo ConnectionInfo;
		ConnectionInfo.DestURL = TEXT("http://localhost:52000"); // Default local URL
		// No API key needed for local provider

		UACEBlueprintLibrary::SetA2XConnectionInfo(ConnectionInfo, LocalProviderName);
	}
}

FName URLA2FComponent::GetCurrentProviderName() const
{
	if (bUseRemoteProvider)
	{
		return RemoteProviderName;
	}
	else
	{
		// Use the specific local provider based on the current model type
		return GetLocalProviderName(LocalModelType);
	}
}

