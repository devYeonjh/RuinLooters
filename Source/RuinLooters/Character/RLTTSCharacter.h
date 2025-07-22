// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RLCharacterBase.h"
#include "ACETypes.h"
#include "TTSxA2F/RLCosyVoiceClient.h"
#include "RLTTSCharacter.generated.h"

// Forward declarations
class URLCosyVoiceTTSComponent;
class URLA2FComponent;
class UACEAudioCurveSourceComponent;

// TTS 음성 시작 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTTSSpeechStarted, const FString&, Text, const FString&, SpeakerID);

// TTS 음성 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTTSSpeechCompleted, const FString&, Text, bool, bSuccess, float, Duration);

// TTS 음성 실패 델리게이트 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTTSSpeechFailed, const FString&, ErrorMessage, const FString&, Text);

/**
 * TTS(Text-to-Speech)와 Audio2Face 얼굴 애니메이션이 통합된 캐릭터 클래스
 * CosyVoice TTS 서버와 ACE A2F를 자동으로 연동하여 음성 + 얼굴 애니메이션 제공
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API ARLTTSCharacter : public ARLCharacterBase
{
	GENERATED_BODY()

public:
	ARLTTSCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ========================================================================================
	// 컴포넌트들
	// ========================================================================================

	// ACE Audio2Face 애니메이션 처리 컴포넌트 (필수)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS Character|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UACEAudioCurveSourceComponent> ACEComponent;

	// TTS 요청 및 A2F 자동 연동 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS Character|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URLCosyVoiceTTSComponent> TTSComponent;

	// Audio2Face 애니메이션 제어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS Character|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URLA2FComponent> A2FComponent;

	// ========================================================================================
	// 설정 및 구성
	// ========================================================================================

	// TTS 서버 기본 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Character|Settings")
	FCosyVoiceSettings DefaultTTSSettings;

	// A2F 감정 파라미터 기본값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Character|Settings")
	FAudio2FaceEmotion DefaultEmotionParams;

	// 기본 화자 ID (빈 문자열이면 서버 기본값 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Character|Settings")
	FString DefaultSpeakerID;

	// A2F 프로바이더 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Character|Settings")
	FName A2FProviderName;

	// 자동 초기화 여부 (BeginPlay에서 자동으로 TTS 서버 연결)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Character|Settings")
	bool bAutoInitializeOnBeginPlay;

	// ========================================================================================
	// 상태 정보
	// ========================================================================================

	// 현재 말하고 있는 텍스트
	UPROPERTY(BlueprintReadOnly, Category = "TTS Character|State")
	FString CurrentSpeechText;

	// 현재 사용중인 화자 ID
	UPROPERTY(BlueprintReadOnly, Category = "TTS Character|State")
	FString CurrentSpeakerID;

	// 마지막 TTS 요청 시작 시간
	UPROPERTY(BlueprintReadOnly, Category = "TTS Character|State")
	float LastSpeechStartTime;

public:
	// ========================================================================================
	// 이벤트 델리게이트
	// ========================================================================================

	// 음성 시작 이벤트
	UPROPERTY(BlueprintAssignable, Category = "TTS Character|Events")
	FOnTTSSpeechStarted OnSpeechStarted;

	// 음성 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "TTS Character|Events")
	FOnTTSSpeechCompleted OnSpeechCompleted;

	// 음성 실패 이벤트
	UPROPERTY(BlueprintAssignable, Category = "TTS Character|Events")
	FOnTTSSpeechFailed OnSpeechFailed;

	// ========================================================================================
	// 주요 음성 기능
	// ========================================================================================

	/**
	 * 텍스트를 음성으로 변환하고 얼굴 애니메이션 재생
	 * @param Text 말할 텍스트
	 * @param SpeakerID 화자 ID (빈 문자열이면 기본값 사용)
	 * @return 요청 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Speech")
	bool SpeakText(const FString& Text, const FString& SpeakerID = TEXT(""));

	/**
	 * 감정 파라미터를 포함하여 텍스트를 음성으로 변환
	 * @param Text 말할 텍스트
	 * @param EmotionParams A2F 감정 파라미터
	 * @param SpeakerID 화자 ID (빈 문자열이면 기본값 사용)
	 * @return 요청 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Speech")
	bool SpeakWithEmotion(const FString& Text, const FAudio2FaceEmotion& EmotionParams, const FString& SpeakerID = TEXT(""));

	/**
	 * 음성만 재생 (얼굴 애니메이션 없음)
	 * @param Text 말할 텍스트
	 * @param SpeakerID 화자 ID (빈 문자열이면 기본값 사용)
	 * @return 요청 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Speech")
	bool SpeakAudioOnly(const FString& Text, const FString& SpeakerID = TEXT(""));

	/**
	 * 현재 진행 중인 음성 중단
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Speech")
	void StopSpeaking();

	// ========================================================================================
	// 상태 확인 함수
	// ========================================================================================

	/**
	 * 현재 말하고 있는지 확인
	 * @return 음성 재생 중이면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|State")
	bool IsSpeaking() const;

	/**
	 * 현재 말하고 있는 텍스트 반환
	 * @return 현재 음성 텍스트
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|State")
	FString GetCurrentSpeechText() const;

	/**
	 * 음성 진행 시간 반환 (초)
	 * @return 현재 음성 시작 후 경과 시간
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|State")
	float GetSpeechElapsedTime() const;

	// ========================================================================================
	// 설정 함수
	// ========================================================================================

	/**
	 * TTS 서버 설정 변경
	 * @param NewSettings 새로운 TTS 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Configuration")
	void SetTTSSettings(const FCosyVoiceSettings& NewSettings);

	/**
	 * 기본 화자 ID 설정
	 * @param NewSpeakerID 새로운 기본 화자 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Configuration")
	void SetDefaultSpeaker(const FString& NewSpeakerID);

	/**
	 * A2F 감정 파라미터 기본값 설정
	 * @param NewEmotionParams 새로운 감정 파라미터
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Configuration")
	void SetDefaultEmotionParams(const FAudio2FaceEmotion& NewEmotionParams);

	/**
	 * A2F 프로바이더 이름 설정
	 * @param NewProviderName 새로운 프로바이더 이름
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Configuration")
	void SetA2FProvider(FName NewProviderName);

	// ========================================================================================
	// 컴포넌트 접근자
	// ========================================================================================

	/**
	 * TTS 컴포넌트 반환
	 * @return TTS 컴포넌트 참조
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Components")
	URLCosyVoiceTTSComponent* GetTTSComponent() const;

	/**
	 * A2F 컴포넌트 반환
	 * @return A2F 컴포넌트 참조
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Components")
	URLA2FComponent* GetA2FComponent() const;

	/**
	 * ACE 컴포넌트 반환
	 * @return ACE 컴포넌트 참조
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS Character|Components")
	UACEAudioCurveSourceComponent* GetACEComponent() const;

private:
	// ========================================================================================
	// 내부 함수
	// ========================================================================================

	// 컴포넌트 초기화
	void InitializeComponents();

	// 이벤트 바인딩
	void BindTTSEvents();

	// 이벤트 언바인딩
	void UnbindTTSEvents();

	// ========================================================================================
	// 이벤트 핸들러
	// ========================================================================================

	// TTS 완료 이벤트 핸들러
	UFUNCTION()
	void OnTTSCompleted(const TArray<uint8>& PCMData, bool bSuccess);

	// TTS 상세 완료 이벤트 핸들러
	UFUNCTION()
	void OnTTSDetailedCompleted(FCosyVoiceResult Result);

	// ========================================================================================
	// 내부 상태
	// ========================================================================================

	// 이벤트 바인딩 상태
	bool bEventsbound;

	// 컴포넌트 초기화 상태
	bool bComponentsInitialized;
};