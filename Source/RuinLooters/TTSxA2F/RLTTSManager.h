#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RLCosyVoiceClient.h"
#include "RLA2FComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/ImportedSoundWave.h"
#include "RLTTSManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTTSCompleted, bool, bSuccess, const FString&, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTTSWithA2FCompleted, bool, bSuccess, const FString&, Text);

USTRUCT(BlueprintType)
struct FTTSRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "TTS Request")
	FString Text;

	UPROPERTY(BlueprintReadWrite, Category = "TTS Request")
	FString SpeakerID;

	UPROPERTY(BlueprintReadWrite, Category = "TTS Request")
	bool bUseA2F = false;

	UPROPERTY(BlueprintReadWrite, Category = "TTS Request")
	bool bAutoPlay = true;

	UPROPERTY(BlueprintReadWrite, Category = "TTS Request")
	float Volume = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "TTS Request")
	float Pitch = 1.0f;

	FTTSRequest()
	{
		Text = TEXT("");
		SpeakerID = TEXT("");
		bUseA2F = false;
		bAutoPlay = true;
		Volume = 1.0f;
		Pitch = 1.0f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLTTSManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URLTTSManager();

protected:
	virtual void BeginPlay() override;

public:
	// TTS Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Manager|Config")
	FString DefaultSpeakerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Manager|Config")
	bool bAutoPlayTTS = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Manager|Config")
	bool bUseA2FByDefault = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Manager|Config")
	float DefaultVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Manager|Config")
	float DefaultPitch = 1.0f;

	// Component references
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS Manager|Components")
	URLCosyVoiceClient* CosyVoiceClient;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS Manager|Components")
	URLA2FComponent* A2FComponent; // Deprecated: Use TargetActor for dynamic A2F

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS Manager|Components")
	UAudioComponent* AudioComponent;

	// Target Actor for dynamic A2F resolution
	UPROPERTY(BlueprintReadOnly, Category = "TTS Manager|Target")
	AActor* TargetActor;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "TTS Manager|Delegates")
	FOnTTSCompleted OnTTSCompleted;

	UPROPERTY(BlueprintAssignable, Category = "TTS Manager|Delegates")
	FOnTTSWithA2FCompleted OnTTSWithA2FCompleted;

	// Main TTS Functions
	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Main")
	void SpeakText(const FString& Text, const FString& SpeakerID = TEXT(""), bool bUseA2F = true);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Main")
	void SpeakTextWithRequest(const FTTSRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Main")
	void StopCurrentTTS();

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Main")
	bool IsCurrentlySpeaking() const;

	// Queue management
	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Queue")
	void QueueTextToSpeak(const FString& Text, const FString& SpeakerID = TEXT(""), bool bUseA2F = true);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Queue")
	void QueueTTSRequest(const FTTSRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Queue")
	void ClearTTSQueue();

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Queue")
	int32 GetQueueLength() const;

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Queue")
	void ProcessNextInQueue();

	// Speaker management
	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Speakers")
	void SetDefaultSpeaker(const FString& SpeakerID);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Speakers")
	void RefreshSpeakerList();

	// Target Actor management
	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Target")
	void SetTargetActor(AActor* NewTargetActor);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Target")
	AActor* GetTargetActor() const { return TargetActor; }

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Target")
	URLA2FComponent* GetTargetA2FComponent() const;

	// Audio control
	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Audio")
	void SetTTSVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Audio")
	void SetTTSPitch(float Pitch);

	UFUNCTION(BlueprintCallable, Category = "TTS Manager|Audio")
	void PlayTTSAudio(UImportedSoundWave* SoundWave, const FTTSRequest& Request);

private:
	// TTS Queue
	TArray<FTTSRequest> TTSQueue;
	bool bIsProcessingTTS = false;
	FTTSRequest CurrentRequest;

	// Event handlers
	UFUNCTION()
	void OnTTSResponseReceived(bool bSuccess, UImportedSoundWave* SoundWave);

	UFUNCTION()
	void OnSpeakerListReceived(bool bSuccess, const TArray<FString>& SpeakerIDs);

	UFUNCTION()
	void OnAudioPlaybackFinished();

	UFUNCTION()
	void OnA2FAnimationCompleted(bool bSuccess);

	// Internal functions
	void ProcessTTSRequest(const FTTSRequest& Request);
	void CompleteTTSRequest(bool bSuccess);
	void SetupAudioComponent();
	void SetupEventBindings();
};