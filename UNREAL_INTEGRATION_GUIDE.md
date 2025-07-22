# CosyVoice 언리얼 엔진 통합 가이드

## 개요
CosyVoice Unreal Server는 언리얼 엔진에서 CosyVoice2 TTS를 사용하기 위한 HTTP REST API 서버입니다.

## 서버 실행

```bash
# 기본 실행 (포트 50001)
cd cosyvoice_unreal_server
python run_server.py

# 옵션 지정
python run_server.py --port 8080 --model CosyVoice2-0.5B
```

## API 엔드포인트

### 1. 화자 관리

**화자 목록 조회**
```
GET /speakers
Response: {
  "speakers": {
    "speaker_id": {
      "prompt_text": "참조 텍스트",
      "prompt_audio": "오디오 파일 경로",
      "description": "설명",
      "created_at": "2025-01-17T10:00:00"
    }
  },
  "default_speaker": "default"
}
```

**화자 추가**
```
POST /speakers
Content-Type: multipart/form-data
- speaker_id: string (필수)
- prompt_text: string (필수)
- prompt_audio: file (필수, WAV 파일)
- description: string (선택)
```

**화자 삭제**
```
DELETE /speakers/{speaker_id}
```

### 2. TTS 생성

**기본 화자 사용**
```
POST /tts
Content-Type: application/json
{
  "text": "합성할 텍스트",
  "return_format": "pcm",  // "pcm" 또는 "wav"
  "sample_rate": 22050
}
```

**특정 화자 사용**
```
POST /tts/speaker
Content-Type: application/json
{
  "text": "합성할 텍스트",
  "speaker_id": "화자ID",
  "return_format": "pcm",
  "sample_rate": 22050
}
```

**일회성 제로샷 (화자 저장 없이)**
```
POST /tts/zero-shot
Content-Type: multipart/form-data
- text: string (필수)
- prompt_text: string (필수)
- prompt_audio: file (필수)
- return_format: string (선택, 기본값 "pcm")
```

### 3. 응답 형식

모든 TTS 엔드포인트는 동일한 형식으로 응답:
```json
{
  "audio_data": "Base64 인코딩된 오디오 데이터",
  "sample_rate": 22050,
  "format": "pcm",
  "channels": 1,
  "bit_depth": 16
}
```

## 언리얼 엔진 통합

### 1. HTTP 클라이언트 설정

**Build.cs**
```cpp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine",
    "Http", "Json", "JsonUtilities"
});
```

### 2. TTS 요청 구현

```cpp
UCLASS()
class YOURGAME_API UCosyVoiceClient : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void GenerateTTS(const FString& Text, const FString& SpeakerID = TEXT(""));
    
    UFUNCTION(BlueprintCallable)
    void CreateVoicePreset(const FString& PresetID, const FString& SampleText, 
                          const TArray<uint8>& AudioData);

private:
    void OnTTSResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};

void UCosyVoiceClient::GenerateTTS(const FString& Text, const FString& SpeakerID)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    
    // 엔드포인트 선택
    FString URL = TEXT("http://localhost:50001");
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
    
    // JSON 페이로드
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("text", Text);
    JsonObject->SetStringField("return_format", "pcm");
    
    if (!SpeakerID.IsEmpty())
    {
        JsonObject->SetStringField("speaker_id", SpeakerID);
    }
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UCosyVoiceClient::OnTTSResponse);
    Request->ProcessRequest();
}
```

### 3. 음성 프리셋 생성

```cpp
void UCosyVoiceClient::CreateVoicePreset(const FString& PresetID, const FString& SampleText, 
                                        const TArray<uint8>& AudioData)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL("http://localhost:50001/speakers");
    Request->SetVerb("POST");
    
    // Multipart form data 생성
    FString Boundary = FGuid::NewGuid().ToString();
    Request->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
    
    // Form data 구성
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
    
    // prompt_audio
    FormData += FString::Printf(TEXT("--%s\r\n"), *Boundary);
    FormData += TEXT("Content-Disposition: form-data; name=\"prompt_audio\"; filename=\"voice.wav\"\r\n");
    FormData += TEXT("Content-Type: audio/wav\r\n\r\n");
    
    // 텍스트를 바이트로 변환
    RequestContent.Append((uint8*)TCHAR_TO_UTF8(*FormData), FormData.Len());
    // 오디오 데이터 추가
    RequestContent.Append(AudioData);
    // 종료 boundary
    FString EndBoundary = FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);
    RequestContent.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());
    
    Request->SetContent(RequestContent);
    Request->ProcessRequest();
}
```

### 4. 응답 처리 및 오디오 재생

```cpp
void UCosyVoiceClient::OnTTSResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (bSuccess && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        // JSON 파싱
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString AudioDataBase64 = JsonObject->GetStringField("audio_data");
            int32 SampleRate = JsonObject->GetIntegerField("sample_rate");
            
            // Base64 디코딩
            TArray<uint8> AudioData;
            FBase64::Decode(AudioDataBase64, AudioData);
            
            // PCM 오디오 재생
            PlayPCMAudio(AudioData, SampleRate);
        }
    }
}

void UCosyVoiceClient::PlayPCMAudio(const TArray<uint8>& PCMData, int32 SampleRate)
{
    USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>();
    SoundWave->SetSampleRate(SampleRate);
    SoundWave->NumChannels = 1;
    SoundWave->Duration = PCMData.Num() / (SampleRate * 2.0f); // 16-bit
    SoundWave->SoundGroup = SOUNDGROUP_Voice;
    
    // PCM 데이터 큐에 추가
    SoundWave->QueueAudio(PCMData.GetData(), PCMData.Num());
    
    // 재생
    UGameplayStatics::PlaySound2D(GetWorld(), SoundWave);
}
```

### 5. 음성 녹음 및 프리셋 생성

```cpp
UCLASS()
class AVoiceRecorder : public AActor
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable)
    void StartRecording(float MaxDuration = 3.0f);
    
    UFUNCTION(BlueprintCallable)
    void StopRecording();
    
    UFUNCTION(BlueprintCallable)
    void SaveAsVoicePreset(const FString& PresetID, const FString& SampleText);
    
private:
    UPROPERTY()
    UAudioCaptureComponent* AudioCapture;
    
    TArray<uint8> RecordedAudio;
    
    void ConvertToWAV(const TArray<float>& AudioData, int32 SampleRate, TArray<uint8>& OutWAV);
};

void AVoiceRecorder::StartRecording(float MaxDuration)
{
    if (!AudioCapture)
    {
        AudioCapture = NewObject<UAudioCaptureComponent>(this);
        AudioCapture->RegisterComponent();
    }
    
    // 녹음 시작
    AudioCapture->Start();
    
    // 최대 시간 후 자동 중지
    GetWorld()->GetTimerManager().SetTimer(RecordingTimer, [this]()
    {
        StopRecording();
    }, MaxDuration, false);
}

void AVoiceRecorder::SaveAsVoicePreset(const FString& PresetID, const FString& SampleText)
{
    if (RecordedAudio.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No audio recorded"));
        return;
    }
    
    // CosyVoice 서버로 전송
    UCosyVoiceClient* Client = NewObject<UCosyVoiceClient>();
    Client->CreateVoicePreset(PresetID, SampleText, RecordedAudio);
}
```

## 사용 예제

### 1. 기본 TTS
```cpp
// 기본 화자로 TTS 생성
CosyVoiceClient->GenerateTTS(TEXT("안녕하세요"));
```

### 2. 특정 화자 사용
```cpp
// 미리 등록된 화자 사용
CosyVoiceClient->GenerateTTS(TEXT("안녕하세요"), TEXT("my_voice"));
```

### 3. 음성 프리셋 생성
```cpp
// 3초 녹음 후 프리셋 생성
VoiceRecorder->StartRecording(3.0f);
// ... 녹음 완료 후 ...
VoiceRecorder->SaveAsVoicePreset(TEXT("player_voice"), TEXT("안녕하세요"));
```

## 주의사항

1. **오디오 형식**: 16-bit PCM, 22050Hz, 모노
2. **Base64 인코딩**: 응답의 audio_data는 Base64로 인코딩됨
3. **화자 저장**: POST /speakers로 추가된 화자는 서버 재시작 후에도 유지됨
4. **최소 녹음 시간**: 제로샷을 위해 최소 0.5초 이상의 오디오 필요
5. **텍스트 길이**: 최대 1000자 제한

## 문제 해결

### 서버 연결 확인
```cpp
// Health check
TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
Request->SetURL("http://localhost:50001/health");
Request->SetVerb("GET");
Request->ProcessRequest();
```

### 화자 목록 확인
```cpp
// 사용 가능한 화자 목록 조회
TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
Request->SetURL("http://localhost:50001/speakers");
Request->SetVerb("GET");
Request->ProcessRequest();
```