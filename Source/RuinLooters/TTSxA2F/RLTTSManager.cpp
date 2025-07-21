#include "RLTTSManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

URLTTSManager::URLTTSManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	TargetActor = nullptr;
}

void URLTTSManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Get or create required components
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("TTSManager: No owner actor found"));
		return;
	}

	// Get CosyVoice client component
	CosyVoiceClient = Owner->FindComponentByClass<URLCosyVoiceClient>();
	if (!CosyVoiceClient)
	{
		UE_LOG(LogTemp, Warning, TEXT("TTSManager: CosyVoice client not found on owner, creating new one"));
		CosyVoiceClient = NewObject<URLCosyVoiceClient>(Owner);
		Owner->AddInstanceComponent(CosyVoiceClient);
		CosyVoiceClient->RegisterComponent();
	}

	// Get A2F component
	A2FComponent = Owner->FindComponentByClass<URLA2FComponent>();
	if (!A2FComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("TTSManager: A2F component not found on owner, A2F features will be disabled"));
		bUseA2FByDefault = false;
	}

	// Get or create audio component
	AudioComponent = Owner->FindComponentByClass<UAudioComponent>();
	if (!AudioComponent)
	{
		AudioComponent = NewObject<UAudioComponent>(Owner);
		Owner->AddInstanceComponent(AudioComponent);
		AudioComponent->RegisterComponent();
	}

	SetupAudioComponent();
	SetupEventBindings();
	
	// Check available speakers on startup
	RefreshSpeakerList();
}

void URLTTSManager::SpeakText(const FString& Text, const FString& SpeakerID, bool bUseA2F)
{
	FTTSRequest Request;
	Request.Text = Text;
	Request.SpeakerID = SpeakerID.IsEmpty() ? DefaultSpeakerID : SpeakerID;
	Request.bUseA2F = bUseA2F && A2FComponent != nullptr;
	Request.bAutoPlay = bAutoPlayTTS;
	Request.Volume = DefaultVolume;
	Request.Pitch = DefaultPitch;

	SpeakTextWithRequest(Request);
}

void URLTTSManager::SpeakTextWithRequest(const FTTSRequest& Request)
{
	if (bIsProcessingTTS)
	{
		// Add to queue if currently processing
		QueueTTSRequest(Request);
		return;
	}

	ProcessTTSRequest(Request);
}

void URLTTSManager::StopCurrentTTS()
{
	// Stop A2F animation if in progress
	if (CurrentRequest.bUseA2F && A2FComponent && A2FComponent->IsAnimationInProgress())
	{
		// Note: A2F may not have a direct stop method, but resetting the component should work
		UE_LOG(LogTemp, Warning, TEXT("TTSManager: Stopping A2F animation - Note: A2F API may not support immediate stop"));
	}
	
	// Stop AudioComponent if playing
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}

	bIsProcessingTTS = false;
	CurrentRequest = FTTSRequest();
}

bool URLTTSManager::IsCurrentlySpeaking() const
{
	if (!bIsProcessingTTS)
	{
		return false;
	}
	
	// Check A2F animation status if using A2F
	if (CurrentRequest.bUseA2F)
	{
		URLA2FComponent* CurrentA2F = GetTargetA2FComponent();
		if (CurrentA2F)
		{
			return CurrentA2F->IsAnimationInProgress();
		}
	}
	
	// Otherwise check AudioComponent
	return AudioComponent && AudioComponent->IsPlaying();
}

void URLTTSManager::QueueTextToSpeak(const FString& Text, const FString& SpeakerID, bool bUseA2F)
{
	FTTSRequest Request;
	Request.Text = Text;
	Request.SpeakerID = SpeakerID.IsEmpty() ? DefaultSpeakerID : SpeakerID;
	Request.bUseA2F = bUseA2F && A2FComponent != nullptr;
	Request.bAutoPlay = bAutoPlayTTS;
	Request.Volume = DefaultVolume;
	Request.Pitch = DefaultPitch;

	QueueTTSRequest(Request);
}

void URLTTSManager::QueueTTSRequest(const FTTSRequest& Request)
{
	TTSQueue.Add(Request);
	UE_LOG(LogTemp, Log, TEXT("TTSManager: Added TTS request to queue. Queue length: %d"), TTSQueue.Num());
}

void URLTTSManager::ClearTTSQueue()
{
	TTSQueue.Empty();
	UE_LOG(LogTemp, Log, TEXT("TTSManager: TTS queue cleared"));
}

int32 URLTTSManager::GetQueueLength() const
{
	return TTSQueue.Num();
}

void URLTTSManager::ProcessNextInQueue()
{
	if (bIsProcessingTTS || TTSQueue.Num() == 0)
	{
		return;
	}

	FTTSRequest NextRequest = TTSQueue[0];
	TTSQueue.RemoveAt(0);
	ProcessTTSRequest(NextRequest);
}

void URLTTSManager::SetDefaultSpeaker(const FString& SpeakerID)
{
	DefaultSpeakerID = SpeakerID;
	UE_LOG(LogTemp, Log, TEXT("TTSManager: Default speaker set to: %s"), *SpeakerID);
}

void URLTTSManager::SetTargetActor(AActor* NewTargetActor)
{
	if (TargetActor != NewTargetActor)
	{
		// Clean up previous A2F event bindings if they exist
		if (TargetActor)
		{
			URLA2FComponent* PrevA2F = TargetActor->FindComponentByClass<URLA2FComponent>();
			if (PrevA2F)
			{
				PrevA2F->OnA2FAnimationCompleted.RemoveDynamic(this, &URLTTSManager::OnA2FAnimationCompleted);
			}
		}

		TargetActor = NewTargetActor;

		// Set up new A2F event bindings
		if (TargetActor)
		{
			URLA2FComponent* NewA2F = TargetActor->FindComponentByClass<URLA2FComponent>();
			if (NewA2F)
			{
				NewA2F->OnA2FAnimationCompleted.AddDynamic(this, &URLTTSManager::OnA2FAnimationCompleted);
				UE_LOG(LogTemp, Log, TEXT("TTSManager: Target actor set to %s with A2F component"), 
					*TargetActor->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TTSManager: Target actor %s has no A2F component"), 
					*TargetActor->GetName());
			}
		}
	}
}

URLA2FComponent* URLTTSManager::GetTargetA2FComponent() const
{
	if (TargetActor)
	{
		return TargetActor->FindComponentByClass<URLA2FComponent>();
	}
	
	// Fallback to legacy A2FComponent for backward compatibility
	return A2FComponent;
}

void URLTTSManager::RefreshSpeakerList()
{
	if (CosyVoiceClient)
	{
		CosyVoiceClient->GetSpeakerList();
	}
}

void URLTTSManager::SetTTSVolume(float Volume)
{
	DefaultVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (AudioComponent)
	{
		AudioComponent->SetVolumeMultiplier(DefaultVolume);
	}
}

void URLTTSManager::SetTTSPitch(float Pitch)
{
	DefaultPitch = FMath::Clamp(Pitch, 0.1f, 2.0f);
	if (AudioComponent)
	{
		AudioComponent->SetPitchMultiplier(DefaultPitch);
	}
}

void URLTTSManager::PlayTTSAudio(UImportedSoundWave* SoundWave, const FTTSRequest& Request)
{
	if (!SoundWave)
	{
		UE_LOG(LogTemp, Error, TEXT("TTSManager: SoundWave is null"));
		CompleteTTSRequest(false);
		return;
	}

	// Get A2F component dynamically (either from TargetActor or legacy A2FComponent)
	URLA2FComponent* CurrentA2F = GetTargetA2FComponent();

	// If A2F is requested and available, use it for both audio and animation
	if (Request.bUseA2F && CurrentA2F)
	{
		// A2F handles audio playback internally
		CurrentA2F->ExecuteA2FAnimationFromSoundWave(SoundWave);
		UE_LOG(LogTemp, Log, TEXT("TTSManager: Started A2F animation with audio for text: %s (Target: %s)"), 
			*Request.Text, TargetActor ? *TargetActor->GetName() : TEXT("Legacy"));
	}
	else if (AudioComponent)
	{
		// Fallback to AudioComponent only when A2F is not used
		AudioComponent->SetSound(SoundWave);
		AudioComponent->SetVolumeMultiplier(Request.Volume);
		AudioComponent->SetPitchMultiplier(Request.Pitch);

		// Bind to playback finished event
		if (UImportedSoundWave* ImportedWave = Cast<UImportedSoundWave>(SoundWave))
		{
			ImportedWave->OnAudioPlaybackFinished.AddDynamic(this, &URLTTSManager::OnAudioPlaybackFinished);
		}

		// Play audio
		AudioComponent->Play();
		UE_LOG(LogTemp, Log, TEXT("TTSManager: Playing TTS audio (non-A2F) for text: %s"), *Request.Text);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TTSManager: No audio playback method available"));
		CompleteTTSRequest(false);
	}
}

void URLTTSManager::OnTTSResponseReceived(bool bSuccess, UImportedSoundWave* SoundWave)
{
	if (bSuccess && SoundWave)
	{
		if (CurrentRequest.bAutoPlay)
		{
			PlayTTSAudio(SoundWave, CurrentRequest);
		}
		else
		{
			CompleteTTSRequest(true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TTSManager: TTS request failed"));
		CompleteTTSRequest(false);
	}
}

void URLTTSManager::OnSpeakerListReceived(bool bSuccess, const TArray<FString>& SpeakerIDs)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TTSManager: Received speaker list with %d speakers"), SpeakerIDs.Num());
		for (const FString& SpeakerID : SpeakerIDs)
		{
			UE_LOG(LogTemp, Log, TEXT("TTSManager: Available speaker: %s"), *SpeakerID);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TTSManager: Failed to retrieve speaker list"));
	}
}

void URLTTSManager::OnAudioPlaybackFinished()
{
	UE_LOG(LogTemp, Log, TEXT("TTSManager: Audio playback finished"));
	CompleteTTSRequest(true);
}

void URLTTSManager::OnA2FAnimationCompleted(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TTSManager: A2F animation completed successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TTSManager: A2F animation failed"));
	}
	
	// Complete the TTS request when A2F animation finishes
	if (CurrentRequest.bUseA2F)
	{
		CompleteTTSRequest(bSuccess);
	}
}

void URLTTSManager::ProcessTTSRequest(const FTTSRequest& Request)
{
	if (!CosyVoiceClient)
	{
		UE_LOG(LogTemp, Error, TEXT("TTSManager: CosyVoice client not available"));
		CompleteTTSRequest(false);
		return;
	}

	if (Request.Text.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TTSManager: Empty text provided"));
		CompleteTTSRequest(false);
		return;
	}

	bIsProcessingTTS = true;
	CurrentRequest = Request;

	UE_LOG(LogTemp, Log, TEXT("TTSManager: Processing TTS request for text: %s"), *Request.Text);
	
	// Use empty speaker ID for default TTS if no specific speaker is provided
	FString SpeakerToUse = Request.SpeakerID;
	
	// If using default speaker ID but it's empty, use empty string for basic TTS
	if (SpeakerToUse.IsEmpty() && !DefaultSpeakerID.IsEmpty())
	{
		SpeakerToUse = DefaultSpeakerID;
		UE_LOG(LogTemp, Log, TEXT("TTSManager: Using default speaker: %s"), *SpeakerToUse);
	}
	else if (SpeakerToUse.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("TTSManager: Using basic TTS (no speaker specified)"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TTSManager: Using specified speaker: %s"), *SpeakerToUse);
	}
	
	// Fallback logic: if speaker fails, try default or empty
	bool bShouldUseFallback = false;
	if (SpeakerToUse == TEXT("sample"))
	{
		UE_LOG(LogTemp, Warning, TEXT("TTSManager: 'sample' speaker might be corrupted, trying 'default' speaker"));
		SpeakerToUse = TEXT("default");
		bShouldUseFallback = true;
	}

	// Generate TTS
	CosyVoiceClient->GenerateTTS(Request.Text, SpeakerToUse);
}

void URLTTSManager::CompleteTTSRequest(bool bSuccess)
{
	FString CompletedText = CurrentRequest.Text;
	bool bWasA2F = CurrentRequest.bUseA2F;

	// Reset processing state
	bIsProcessingTTS = false;
	CurrentRequest = FTTSRequest();

	// Broadcast completion
	OnTTSCompleted.Broadcast(bSuccess, CompletedText);
	
	if (bWasA2F)
	{
		OnTTSWithA2FCompleted.Broadcast(bSuccess, CompletedText);
	}

	// Process next in queue if available
	if (TTSQueue.Num() > 0)
	{
		// Use a small delay to prevent immediate processing
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &URLTTSManager::ProcessNextInQueue, 0.1f, false);
	}
}

void URLTTSManager::SetupAudioComponent()
{
	if (!AudioComponent)
	{
		return;
	}

	AudioComponent->SetVolumeMultiplier(DefaultVolume);
	AudioComponent->SetPitchMultiplier(DefaultPitch);
	AudioComponent->bAutoActivate = false;
	AudioComponent->bStopWhenOwnerDestroyed = true;
}

void URLTTSManager::SetupEventBindings()
{
	if (CosyVoiceClient)
	{
		CosyVoiceClient->OnTTSResponse.AddDynamic(this, &URLTTSManager::OnTTSResponseReceived);
		CosyVoiceClient->OnSpeakerListResponse.AddDynamic(this, &URLTTSManager::OnSpeakerListReceived);
	}
	
	// Legacy A2F binding removed - now handled dynamically in SetTargetActor()
	// if (A2FComponent)
	// {
	//     A2FComponent->OnA2FAnimationCompleted.AddDynamic(this, &URLTTSManager::OnA2FAnimationCompleted);
	// }
}