#include "RLCosyVoiceTTSComponent.h"
#include "RLA2FComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogRLCosyVoiceTTSComponent, Log, All);

URLCosyVoiceTTSComponent::URLCosyVoiceTTSComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	VoiceClient = nullptr;
	CachedA2FComponent = nullptr;
	bAutoFindA2FComponent = true;
}

void URLCosyVoiceTTSComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeVoiceClient();
	
	// Cache A2F component if auto-find is enabled
	if (bAutoFindA2FComponent)
	{
		CachedA2FComponent = FindA2FComponent();
		if (CachedA2FComponent)
		{
			UE_LOG(LogRLCosyVoiceTTSComponent, Log, TEXT("Found A2F component on actor: %s"), 
				   *GetOwner()->GetName());
		}
		else
		{
			UE_LOG(LogRLCosyVoiceTTSComponent, Warning, TEXT("No A2F component found on actor: %s"), 
				   *GetOwner()->GetName());
		}
	}
}

void URLCosyVoiceTTSComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (VoiceClient)
	{
		// Unbind delegates
		VoiceClient->OnTTSComplete.RemoveDynamic(this, &URLCosyVoiceTTSComponent::OnVoiceClientTTSComplete);
		VoiceClient->OnTTSCompleteDetailed.RemoveDynamic(this, &URLCosyVoiceTTSComponent::OnVoiceClientTTSCompleteDetailed);
		
		// Cancel any ongoing requests
		if (VoiceClient->IsRequestInProgress())
		{
			VoiceClient->CancelCurrentRequest();
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void URLCosyVoiceTTSComponent::SpeakText(const FString& Text, const FString& SpeakerID)
{
	if (!VoiceClient)
	{
		UE_LOG(LogRLCosyVoiceTTSComponent, Error, TEXT("SpeakText: VoiceClient is not initialized"));
		return;
	}
	
	if (Text.IsEmpty())
	{
		UE_LOG(LogRLCosyVoiceTTSComponent, Warning, TEXT("SpeakText: Text is empty"));
		return;
	}
	
	UE_LOG(LogRLCosyVoiceTTSComponent, Log, TEXT("Speaking text: %s"), *Text);
	VoiceClient->GenerateTTS(Text, SpeakerID);
}

void URLCosyVoiceTTSComponent::SpeakTextWithFacialAnimation(const FString& Text, const FString& SpeakerID)
{
	if (!VoiceClient)
	{
		UE_LOG(LogRLCosyVoiceTTSComponent, Error, TEXT("SpeakTextWithFacialAnimation: VoiceClient is not initialized"));
		return;
	}
	
	if (Text.IsEmpty())
	{
		UE_LOG(LogRLCosyVoiceTTSComponent, Warning, TEXT("SpeakTextWithFacialAnimation: Text is empty"));
		return;
	}
	
	URLA2FComponent* A2FComponent = GetA2FComponent();
	if (!A2FComponent)
	{
		UE_LOG(LogRLCosyVoiceTTSComponent, Error, TEXT("SpeakTextWithFacialAnimation: No A2F component available"));
		
		// Fallback to audio-only TTS
		SpeakText(Text, SpeakerID);
		return;
	}
	
	UE_LOG(LogRLCosyVoiceTTSComponent, Log, TEXT("Speaking text with facial animation: %s"), *Text);
	VoiceClient->GenerateTTSWithA2F(Text, A2FComponent, SpeakerID);
}

bool URLCosyVoiceTTSComponent::IsRequestInProgress() const
{
	return VoiceClient ? VoiceClient->IsRequestInProgress() : false;
}

void URLCosyVoiceTTSComponent::CancelCurrentRequest()
{
	if (VoiceClient)
	{
		VoiceClient->CancelCurrentRequest();
	}
}

URLA2FComponent* URLCosyVoiceTTSComponent::GetA2FComponent()
{
	if (bAutoFindA2FComponent)
	{
		// Use cached component if available and valid
		if (CachedA2FComponent && IsValid(CachedA2FComponent))
		{
			return CachedA2FComponent;
		}
		
		// Refresh cache
		CachedA2FComponent = FindA2FComponent();
		return CachedA2FComponent;
	}
	else
	{
		// Use manually specified component
		return ManualA2FComponent.Get();
	}
}

void URLCosyVoiceTTSComponent::OnVoiceClientTTSComplete(const TArray<uint8>& PCMData, bool bSuccess)
{
	// Forward the event
	OnTTSComplete.Broadcast(PCMData, bSuccess);
}

void URLCosyVoiceTTSComponent::OnVoiceClientTTSCompleteDetailed(FCosyVoiceResult Result)
{
	// Forward the event
	OnTTSCompleteDetailed.Broadcast(Result);
}

void URLCosyVoiceTTSComponent::InitializeVoiceClient()
{
	if (VoiceClient)
	{
		return; // Already initialized
	}
	
	VoiceClient = NewObject<URLCosyVoiceClient>(this);
	if (!VoiceClient)
	{
		UE_LOG(LogRLCosyVoiceTTSComponent, Error, TEXT("Failed to create VoiceClient"));
		return;
	}
	
	// Copy settings
	VoiceClient->Settings = VoiceSettings;
	
	// Bind events
	VoiceClient->OnTTSComplete.AddDynamic(this, &URLCosyVoiceTTSComponent::OnVoiceClientTTSComplete);
	VoiceClient->OnTTSCompleteDetailed.AddDynamic(this, &URLCosyVoiceTTSComponent::OnVoiceClientTTSCompleteDetailed);
	
	UE_LOG(LogRLCosyVoiceTTSComponent, Log, TEXT("VoiceClient initialized successfully"));
}

URLA2FComponent* URLCosyVoiceTTSComponent::FindA2FComponent()
{
	if (!GetOwner())
	{
		return nullptr;
	}
	
	// Look for A2F component on the same actor
	URLA2FComponent* A2FComponent = GetOwner()->GetComponentByClass<URLA2FComponent>();
	if (A2FComponent)
	{
		return A2FComponent;
	}
	
	// Look for A2F component on child actors (for complex character setups)
	TArray<AActor*> AttachedActors;
	GetOwner()->GetAttachedActors(AttachedActors);
	
	for (AActor* AttachedActor : AttachedActors)
	{
		if (AttachedActor)
		{
			A2FComponent = AttachedActor->GetComponentByClass<URLA2FComponent>();
			if (A2FComponent)
			{
				UE_LOG(LogRLCosyVoiceTTSComponent, Log, TEXT("Found A2F component on attached actor: %s"), 
					   *AttachedActor->GetName());
				return A2FComponent;
			}
		}
	}
	
	return nullptr;
}