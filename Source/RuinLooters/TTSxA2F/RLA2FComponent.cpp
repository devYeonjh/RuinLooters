#include "RLA2FComponent.h"
#include "ACEBlueprintLibrary.h"
#include "Engine/World.h"

URLA2FComponent::URLA2FComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentAsyncAction = nullptr;
}

void URLA2FComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeDefaultParameters();
}

void URLA2FComponent::ExecuteA2FAnimation(const FString& WavFilePath)
{
	if (WavFilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: WavFilePath is empty"));
		return;
	}

	if (CurrentAsyncAction)
	{
		CurrentAsyncAction->AudioSendCompleted.RemoveDynamic(this, &URLA2FComponent::OnAnimationCompleted);
		CurrentAsyncAction = nullptr;
	}

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	CurrentAsyncAction = UAsyncActionAnimateCharacter::AnimateCharacterFromWavFileAsync(
		GetWorld(),
		GetOwner(),
		WavFilePath,
		EmotionParams,
		FaceParams,
		ProviderName
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

	if (CurrentAsyncAction)
	{
		CurrentAsyncAction->AudioSendCompleted.RemoveDynamic(this, &URLA2FComponent::OnAnimationCompleted);
		CurrentAsyncAction = nullptr;
	}

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	CurrentAsyncAction = UAsyncActionAnimateCharacter::AnimateCharacterFromSoundWaveAsync(
		GetWorld(),
		GetOwner(),
		SoundWave,
		EmotionParams,
		FaceParams,
		ProviderName
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

	if (!FaceParams)
	{
		FaceParams = UACEBlueprintLibrary::CreateAudio2FaceParameters(GetWorld());
	}

	// Use synchronous version from UACEBlueprintLibrary
	bool bSuccess = UACEBlueprintLibrary::AnimateCharacterFromSoundWave(
		GetOwner(),
		SoundWave,
		EmotionParams,
		FaceParams,
		ProviderName
	);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: Sync animation failed"));
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

	// Debug: Log available providers
	TArray<FName> AvailableProviders = UACEBlueprintLibrary::GetAvailableA2FProviderNames();
	UE_LOG(LogTemp, Warning, TEXT("RLA2FComponent: Available A2F Providers:"));
	for (const FName& Provider : AvailableProviders)
	{
		UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Provider.ToString());
	}
}