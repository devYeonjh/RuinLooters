// Fill out your copyright notice in the Description page of Project Settings.

#include "RLTTSCharacter.h"
#include "TTSxA2F/RLCosyVoiceTTSComponent.h"
#include "TTSxA2F/RLA2FComponent.h"
#include "ACEAudioCurveSourceComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRLTTSCharacter, Log, All);

ARLTTSCharacter::ARLTTSCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 생성
	ACEComponent = CreateDefaultSubobject<UACEAudioCurveSourceComponent>(TEXT("ACEComponent"));
	TTSComponent = CreateDefaultSubobject<URLCosyVoiceTTSComponent>(TEXT("TTSComponent"));
	A2FComponent = CreateDefaultSubobject<URLA2FComponent>(TEXT("A2FComponent"));

	// 기본 설정 초기화
	DefaultTTSSettings.ServerURL = TEXT("http://localhost:8000");
	DefaultTTSSettings.RequestTimeout = 30.0f;
	DefaultTTSSettings.SampleRate = 22050;
	DefaultTTSSettings.bLogVerbose = true;

	// 기본 A2F 감정 파라미터 (구조체의 기본값 사용)
	DefaultEmotionParams.OverallEmotionStrength = 0.6f;
	DefaultEmotionParams.DetectedEmotionContrast = 1.0f;
	DefaultEmotionParams.MaxDetectedEmotions = 3;
	DefaultEmotionParams.DetectedEmotionSmoothing = 0.7f;
	DefaultEmotionParams.bEnableEmotionOverride = true;
	DefaultEmotionParams.EmotionOverrideStrength = 0.5f;

	// 기본값 설정
	DefaultSpeakerID = TEXT("");
	A2FProviderName = TEXT("Default");
	bAutoInitializeOnBeginPlay = true;

	// 상태 초기화
	CurrentSpeechText = TEXT("");
	CurrentSpeakerID = TEXT("");
	LastSpeechStartTime = 0.0f;
	bEventsbound = false;
	bComponentsInitialized = false;
}

void ARLTTSCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogRLTTSCharacter, Log, TEXT("RLTTSCharacter BeginPlay: %s"), *GetName());

	// 컴포넌트 초기화
	InitializeComponents();

	// 이벤트 바인딩
	BindTTSEvents();

	// 자동 초기화
	if (bAutoInitializeOnBeginPlay)
	{
		UE_LOG(LogRLTTSCharacter, Log, TEXT("Auto-initializing TTS character: %s"), *GetName());
	}
}

void ARLTTSCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 진행 중인 음성 중단
	StopSpeaking();

	// 이벤트 언바인딩
	UnbindTTSEvents();

	Super::EndPlay(EndPlayReason);

	UE_LOG(LogRLTTSCharacter, Log, TEXT("RLTTSCharacter EndPlay: %s"), *GetName());
}

void ARLTTSCharacter::InitializeComponents()
{
	if (bComponentsInitialized)
	{
		return;
	}

	// 컴포넌트 유효성 검사
	if (!ACEComponent)
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("ACEComponent is null in %s"), *GetName());
		return;
	}

	if (!TTSComponent)
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("TTSComponent is null in %s"), *GetName());
		return;
	}

	if (!A2FComponent)
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("A2FComponent is null in %s"), *GetName());
		return;
	}

	// TTS 컴포넌트 설정
	TTSComponent->VoiceSettings = DefaultTTSSettings;
	TTSComponent->bAutoFindA2FComponent = true; // A2F 컴포넌트 자동 검색

	// A2F 컴포넌트 설정
	A2FComponent->EmotionParams = DefaultEmotionParams;
	A2FComponent->ProviderName = A2FProviderName;

	bComponentsInitialized = true;
	UE_LOG(LogRLTTSCharacter, Log, TEXT("Components initialized successfully for %s"), *GetName());
}

void ARLTTSCharacter::BindTTSEvents()
{
	if (bEventsbound || !TTSComponent)
	{
		return;
	}

	// TTS 이벤트 바인딩
	TTSComponent->OnTTSComplete.AddDynamic(this, &ARLTTSCharacter::OnTTSCompleted);
	TTSComponent->OnTTSCompleteDetailed.AddDynamic(this, &ARLTTSCharacter::OnTTSDetailedCompleted);

	bEventsbound = true;
	UE_LOG(LogRLTTSCharacter, Log, TEXT("TTS events bound for %s"), *GetName());
}

void ARLTTSCharacter::UnbindTTSEvents()
{
	if (!bEventsbound || !TTSComponent)
	{
		return;
	}

	// TTS 이벤트 언바인딩
	TTSComponent->OnTTSComplete.RemoveDynamic(this, &ARLTTSCharacter::OnTTSCompleted);
	TTSComponent->OnTTSCompleteDetailed.RemoveDynamic(this, &ARLTTSCharacter::OnTTSDetailedCompleted);

	bEventsbound = false;
	UE_LOG(LogRLTTSCharacter, Log, TEXT("TTS events unbound for %s"), *GetName());
}

bool ARLTTSCharacter::SpeakText(const FString& Text, const FString& SpeakerID)
{
	if (!TTSComponent)
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("SpeakText: TTSComponent is null in %s"), *GetName());
		return false;
	}

	if (Text.IsEmpty())
	{
		UE_LOG(LogRLTTSCharacter, Warning, TEXT("SpeakText: Empty text provided to %s"), *GetName());
		return false;
	}

	// 이미 진행 중인 요청이 있으면 중단
	if (IsSpeaking())
	{
		UE_LOG(LogRLTTSCharacter, Log, TEXT("SpeakText: Stopping current speech before starting new one in %s"), *GetName());
		StopSpeaking();
	}

	// 사용할 화자 ID 결정
	FString UseSpeakerID = SpeakerID.IsEmpty() ? DefaultSpeakerID : SpeakerID;

	// 상태 업데이트
	CurrentSpeechText = Text;
	CurrentSpeakerID = UseSpeakerID;
	LastSpeechStartTime = GetWorld()->GetTimeSeconds();

	// 음성 시작 이벤트 발생
	OnSpeechStarted.Broadcast(Text, UseSpeakerID);

	// TTS + A2F 요청
	TTSComponent->SpeakTextWithFacialAnimation(Text, UseSpeakerID);

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SpeakText: Started speech for %s - Text: %s, Speaker: %s"), 
		   *GetName(), *Text, *UseSpeakerID);

	return true;
}

bool ARLTTSCharacter::SpeakWithEmotion(const FString& Text, const FAudio2FaceEmotion& EmotionParams, const FString& SpeakerID)
{
	if (!A2FComponent)
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("SpeakWithEmotion: A2FComponent is null in %s"), *GetName());
		return false;
	}

	// 감정 파라미터 임시 설정
	FAudio2FaceEmotion OriginalParams = A2FComponent->EmotionParams;
	A2FComponent->EmotionParams = EmotionParams;

	// 일반적인 음성 재생
	bool bResult = SpeakText(Text, SpeakerID);

	// 원래 감정 파라미터로 복원 (다음 프레임에)
	GetWorld()->GetTimerManager().SetTimerForNextTick([this, OriginalParams]()
	{
		if (A2FComponent)
		{
			A2FComponent->EmotionParams = OriginalParams;
		}
	});

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SpeakWithEmotion: Started emotional speech for %s - Emotion Strength: %.2f"), 
		   *GetName(), EmotionParams.OverallEmotionStrength);

	return bResult;
}

bool ARLTTSCharacter::SpeakAudioOnly(const FString& Text, const FString& SpeakerID)
{
	if (!TTSComponent)
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("SpeakAudioOnly: TTSComponent is null in %s"), *GetName());
		return false;
	}

	if (Text.IsEmpty())
	{
		UE_LOG(LogRLTTSCharacter, Warning, TEXT("SpeakAudioOnly: Empty text provided to %s"), *GetName());
		return false;
	}

	// 이미 진행 중인 요청이 있으면 중단
	if (IsSpeaking())
	{
		StopSpeaking();
	}

	// 사용할 화자 ID 결정
	FString UseSpeakerID = SpeakerID.IsEmpty() ? DefaultSpeakerID : SpeakerID;

	// 상태 업데이트
	CurrentSpeechText = Text;
	CurrentSpeakerID = UseSpeakerID;
	LastSpeechStartTime = GetWorld()->GetTimeSeconds();

	// 음성 시작 이벤트 발생
	OnSpeechStarted.Broadcast(Text, UseSpeakerID);

	// 음성만 재생 (A2F 없음)
	TTSComponent->SpeakText(Text, UseSpeakerID);

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SpeakAudioOnly: Started audio-only speech for %s"), *GetName());

	return true;
}

void ARLTTSCharacter::StopSpeaking()
{
	if (!TTSComponent)
	{
		return;
	}

	// TTS 요청 취소
	TTSComponent->CancelCurrentRequest();

	// 상태 초기화
	CurrentSpeechText = TEXT("");
	CurrentSpeakerID = TEXT("");
	LastSpeechStartTime = 0.0f;

	UE_LOG(LogRLTTSCharacter, Log, TEXT("StopSpeaking: Speech stopped for %s"), *GetName());
}

bool ARLTTSCharacter::IsSpeaking() const
{
	return TTSComponent ? TTSComponent->IsRequestInProgress() : false;
}

FString ARLTTSCharacter::GetCurrentSpeechText() const
{
	return CurrentSpeechText;
}

float ARLTTSCharacter::GetSpeechElapsedTime() const
{
	if (!IsSpeaking() || LastSpeechStartTime <= 0.0f)
	{
		return 0.0f;
	}

	return GetWorld()->GetTimeSeconds() - LastSpeechStartTime;
}

void ARLTTSCharacter::SetTTSSettings(const FCosyVoiceSettings& NewSettings)
{
	DefaultTTSSettings = NewSettings;
	
	if (TTSComponent)
	{
		TTSComponent->VoiceSettings = NewSettings;
	}

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SetTTSSettings: Updated TTS settings for %s - Server: %s"), 
		   *GetName(), *NewSettings.ServerURL);
}

void ARLTTSCharacter::SetDefaultSpeaker(const FString& NewSpeakerID)
{
	DefaultSpeakerID = NewSpeakerID;

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SetDefaultSpeaker: Updated default speaker for %s - Speaker: %s"), 
		   *GetName(), *NewSpeakerID);
}

void ARLTTSCharacter::SetDefaultEmotionParams(const FAudio2FaceEmotion& NewEmotionParams)
{
	DefaultEmotionParams = NewEmotionParams;
	
	if (A2FComponent)
	{
		A2FComponent->EmotionParams = NewEmotionParams;
	}

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SetDefaultEmotionParams: Updated emotion params for %s - Strength: %.2f"), 
		   *GetName(), NewEmotionParams.OverallEmotionStrength);
}

void ARLTTSCharacter::SetA2FProvider(FName NewProviderName)
{
	A2FProviderName = NewProviderName;
	
	if (A2FComponent)
	{
		A2FComponent->ProviderName = NewProviderName;
	}

	UE_LOG(LogRLTTSCharacter, Log, TEXT("SetA2FProvider: Updated A2F provider for %s - Provider: %s"), 
		   *GetName(), *NewProviderName.ToString());
}

URLCosyVoiceTTSComponent* ARLTTSCharacter::GetTTSComponent() const
{
	return TTSComponent;
}

URLA2FComponent* ARLTTSCharacter::GetA2FComponent() const
{
	return A2FComponent;
}

UACEAudioCurveSourceComponent* ARLTTSCharacter::GetACEComponent() const
{
	return ACEComponent;
}

void ARLTTSCharacter::OnTTSCompleted(const TArray<uint8>& PCMData, bool bSuccess)
{
	float Duration = GetSpeechElapsedTime();
	FString CompletedText = CurrentSpeechText;

	if (bSuccess)
	{
		UE_LOG(LogRLTTSCharacter, Log, TEXT("OnTTSCompleted: Speech completed successfully for %s - Duration: %.2fs, Data: %d bytes"), 
			   *GetName(), Duration, PCMData.Num());
		
		OnSpeechCompleted.Broadcast(CompletedText, true, Duration);
	}
	else
	{
		UE_LOG(LogRLTTSCharacter, Warning, TEXT("OnTTSCompleted: Speech failed for %s - Duration: %.2fs"), 
			   *GetName(), Duration);
		
		OnSpeechCompleted.Broadcast(CompletedText, false, Duration);
		OnSpeechFailed.Broadcast(TEXT("TTS generation failed"), CompletedText);
	}

	// 상태 초기화
	CurrentSpeechText = TEXT("");
	CurrentSpeakerID = TEXT("");
	LastSpeechStartTime = 0.0f;
}

void ARLTTSCharacter::OnTTSDetailedCompleted(FCosyVoiceResult Result)
{
	if (Result.bSuccess)
	{
		UE_LOG(LogRLTTSCharacter, Verbose, TEXT("OnTTSDetailedCompleted: Success - Processing time: %.2fs, Sample rate: %d Hz, Channels: %d"), 
			   Result.ProcessingTime, Result.SampleRate, Result.NumChannels);
	}
	else
	{
		UE_LOG(LogRLTTSCharacter, Error, TEXT("OnTTSDetailedCompleted: Failed - Error: %s"), 
			   *Result.ErrorMessage);
		
		// 상세 실패 이벤트 발생
		OnSpeechFailed.Broadcast(Result.ErrorMessage, CurrentSpeechText);
	}
}