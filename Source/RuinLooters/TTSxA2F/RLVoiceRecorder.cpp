#include "RLVoiceRecorder.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "RLCosyVoiceClient.h"

ARLVoiceRecorder::ARLVoiceRecorder()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARLVoiceRecorder::BeginPlay()
{
	Super::BeginPlay();

	// Create audio importer instance
	AudioImporter = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
	
	UE_LOG(LogTemp, Log, TEXT("VoiceRecorder: Initialized for basic recording simulation"));
}

void ARLVoiceRecorder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop recording if active
	if (bIsRecording)
	{
		StopRecording();
	}

	Super::EndPlay(EndPlayReason);
}

void ARLVoiceRecorder::StartRecording(float MaxDuration)
{
	if (bIsRecording)
	{
		UE_LOG(LogTemp, Warning, TEXT("Recording already in progress"));
		return;
	}

	// Remove the audio capture check since we're using simulation

	MaxRecordingDuration = MaxDuration;
	bIsRecording = true;
	bIsCapturingAudio = true;
	RecordingStartTime = GetWorld()->GetTimeSeconds();
	RecordedAudio.Empty();
	CapturedFloatData.Empty();

	// Start simulated audio capture
	GenerateSampleAudioData();

	// Set timer for maximum duration
	GetWorld()->GetTimerManager().SetTimer(RecordingTimer, this, &ARLVoiceRecorder::OnRecordingTimerExpired, MaxDuration, false);

	// Broadcast recording started
	OnRecordingStarted.Broadcast(MaxDuration);

	UE_LOG(LogTemp, Log, TEXT("Voice recording started for %.2f seconds"), MaxDuration);
}

void ARLVoiceRecorder::StopRecording()
{
	if (!bIsRecording)
	{
		UE_LOG(LogTemp, Warning, TEXT("No recording in progress"));
		return;
	}

	bIsRecording = false;

	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(RecordingTimer);

	// Stop simulated audio capture
	bIsCapturingAudio = false;
	ProcessRecordedAudio();

	// Broadcast recording stopped
	OnRecordingStopped.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Voice recording stopped"));
}

bool ARLVoiceRecorder::IsRecording() const
{
	return bIsRecording;
}

float ARLVoiceRecorder::GetRecordingDuration() const
{
	if (!bIsRecording)
	{
		return 0.0f;
	}

	return GetWorld()->GetTimeSeconds() - RecordingStartTime;
}

void ARLVoiceRecorder::SaveAsVoicePreset(const FString& PresetID, const FString& SampleText, const FString& Description)
{
	if (RecordedAudio.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No audio recorded"));
		return;
	}

	// Find CosyVoice client component in the world
	// This is a simple implementation - in a real scenario, you might want to have a more robust way to find or inject the client
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (URLCosyVoiceClient* CosyVoiceClient = Actor->FindComponentByClass<URLCosyVoiceClient>())
		{
			CosyVoiceClient->CreateVoicePreset(PresetID, SampleText, RecordedAudio, Description);
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("CosyVoice client component not found in the world"));
}

void ARLVoiceRecorder::OnRecordingTimerExpired()
{
	StopRecording();
}

void ARLVoiceRecorder::ProcessRecordedAudio()
{
	if (CapturedFloatData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No audio data captured"));
		return;
	}

	// Convert to WAV format
	RecordedAudio = ConvertToWAV(CapturedFloatData, SampleRate, NumChannels);

	// Broadcast completion
	OnRecordingComplete.Broadcast(RecordedAudio);

	UE_LOG(LogTemp, Log, TEXT("Audio processing complete. Recorded %d bytes"), RecordedAudio.Num());
}

TArray<uint8> ARLVoiceRecorder::ConvertToWAV(const TArray<float>& AudioData, int32 InSampleRate, int32 InNumChannels)
{
	TArray<uint8> WAVData;

	// Calculate data size
	int32 DataSize = AudioData.Num() * sizeof(int16);
	int32 FileSize = 44 + DataSize;

	// WAV header
	WAVData.Reserve(FileSize);

	// RIFF header
	WAVData.Append((uint8*)"RIFF", 4);
	WAVData.Append((uint8*)&FileSize, 4);
	WAVData.Append((uint8*)"WAVE", 4);

	// fmt chunk
	WAVData.Append((uint8*)"fmt ", 4);
	int32 FmtChunkSize = 16;
	WAVData.Append((uint8*)&FmtChunkSize, 4);
	int16 AudioFormat = 1; // PCM
	WAVData.Append((uint8*)&AudioFormat, 2);
	int16 NumChannels16 = (int16)InNumChannels;
	WAVData.Append((uint8*)&NumChannels16, 2);
	WAVData.Append((uint8*)&InSampleRate, 4);
	int32 ByteRate = InSampleRate * InNumChannels * 2;
	WAVData.Append((uint8*)&ByteRate, 4);
	int16 BlockAlign = InNumChannels * 2;
	WAVData.Append((uint8*)&BlockAlign, 2);
	int16 BitsPerSample16 = 16;
	WAVData.Append((uint8*)&BitsPerSample16, 2);

	// data chunk
	WAVData.Append((uint8*)"data", 4);
	WAVData.Append((uint8*)&DataSize, 4);

	// Convert float samples to 16-bit PCM
	for (float Sample : AudioData)
	{
		int16 PCMSample = (int16)(FMath::Clamp(Sample, -1.0f, 1.0f) * 32767.0f);
		WAVData.Append((uint8*)&PCMSample, 2);
	}

	return WAVData;
}

void ARLVoiceRecorder::GenerateSampleAudioData()
{
	// Generate sample sine wave audio data for testing
	float Duration = FMath::Min(MaxRecordingDuration, 3.0f);
	int32 NumSamples = SampleRate * Duration * NumChannels;
	
	CapturedFloatData.Empty();
	CapturedFloatData.Reserve(NumSamples);
	
	float Frequency = 440.0f; // A note
	
	for (int32 i = 0; i < NumSamples; i++)
	{
		float Time = static_cast<float>(i) / SampleRate;
		float SampleValue = FMath::Sin(2.0f * PI * Frequency * Time) * 0.5f; // 50% volume
		CapturedFloatData.Add(SampleValue);
	}
	
	UE_LOG(LogTemp, Log, TEXT("VoiceRecorder: Generated %d sample audio data points"), NumSamples);
}