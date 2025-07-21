#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Sound/SoundSubmix.h"
#include "RuntimeAudioImporterLibrary.h"
#include "RLVoiceRecorder.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingComplete, const TArray<uint8>&, AudioData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingStarted, float, MaxDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStopped);

UCLASS()
class RUINLOOTERS_API ARLVoiceRecorder : public AActor
{
	GENERATED_BODY()

public:
	ARLVoiceRecorder();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Recording controls
	UFUNCTION(BlueprintCallable, Category = "Voice Recorder")
	void StartRecording(float MaxDuration = 3.0f);

	UFUNCTION(BlueprintCallable, Category = "Voice Recorder")
	void StopRecording();

	UFUNCTION(BlueprintCallable, Category = "Voice Recorder")
	bool IsRecording() const;

	UFUNCTION(BlueprintCallable, Category = "Voice Recorder")
	float GetRecordingDuration() const;

	// Voice preset creation
	UFUNCTION(BlueprintCallable, Category = "Voice Recorder")
	void SaveAsVoicePreset(const FString& PresetID, const FString& SampleText, const FString& Description = TEXT(""));

	// Audio configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Recorder|Config")
	int32 SampleRate = 22050;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Recorder|Config")
	int32 NumChannels = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Recorder|Config")
	int32 BitsPerSample = 16;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Voice Recorder|Delegates")
	FOnRecordingComplete OnRecordingComplete;

	UPROPERTY(BlueprintAssignable, Category = "Voice Recorder|Delegates")
	FOnRecordingStarted OnRecordingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Voice Recorder|Delegates")
	FOnRecordingStopped OnRecordingStopped;

private:
	// Simple recording simulation using timer-based approach
	bool bIsCapturingAudio = false;

	// Timer for maximum recording duration
	FTimerHandle RecordingTimer;

	// Recording state
	bool bIsRecording = false;
	float RecordingStartTime = 0.0f;
	float MaxRecordingDuration = 3.0f;

	// Recorded audio data
	TArray<uint8> RecordedAudio;
	TArray<float> CapturedFloatData;

	// Audio importer for format conversion
	UPROPERTY()
	URuntimeAudioImporterLibrary* AudioImporter;

	// Internal functions
	void OnRecordingTimerExpired();
	void ProcessRecordedAudio();
	TArray<uint8> ConvertToWAV(const TArray<float>& AudioData, int32 InSampleRate, int32 InNumChannels);
	
	// Generate sample audio data for testing
	void GenerateSampleAudioData();
};