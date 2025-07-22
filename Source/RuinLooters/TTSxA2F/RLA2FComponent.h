#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ACETypes.h"
#include "Audio2FaceParameters.h"
#include "AsyncActionAnimateCharacter.h"
#include "RLA2FComponent.generated.h"

UENUM(BlueprintType)
enum class EA2FProviderType : uint8
{
	Remote UMETA(DisplayName = "Remote (API-based)"),
	Local UMETA(DisplayName = "Local (GPU-based)")
};

UENUM(BlueprintType)
enum class EA2FLocalModel : uint8
{
	Mark UMETA(DisplayName = "Mark (Expressive)"),
	Claire UMETA(DisplayName = "Claire (Mandarin optimized)"),
	James UMETA(DisplayName = "James (Strong articulation)")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLA2FComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URLA2FComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "RLA2F")
	void ExecuteA2FAnimation(const FString& WavFilePath);

	UFUNCTION(BlueprintCallable, Category = "RLA2F")
	void ExecuteA2FAnimationFromSoundWave(USoundWave* SoundWave);

	UFUNCTION(BlueprintCallable, Category = "RLA2F")
	bool ExecuteA2FAnimationFromSoundWaveSync(USoundWave* SoundWave);

	// Direct PCM data animation (bypasses SoundWave creation)
	UFUNCTION(BlueprintCallable, Category = "RLA2F")
	bool ExecuteA2FAnimationFromPCMDataSync(const TArray<uint8>& PCMData, int32 SampleRate, int32 NumChannels = 1);

	// Provider configuration methods
	UFUNCTION(BlueprintCallable, Category = "RLA2F|Provider")
	bool SetupAPIProvider(const FString& ServerURL, const FString& APIKey, const FString& FunctionId = "", const FString& FunctionVersion = "");

	UFUNCTION(BlueprintCallable, Category = "RLA2F|Provider")
	bool SetupLocalProvider(EA2FLocalModel ModelType = EA2FLocalModel::Mark);

	UFUNCTION(BlueprintCallable, Category = "RLA2F|Provider")
	TArray<FString> GetAvailableProviders() const;

	UFUNCTION(BlueprintCallable, Category = "RLA2F|Provider")
	bool IsLocalProviderAvailable(EA2FLocalModel ModelType) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F")
	FAudio2FaceEmotion EmotionParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F")
	UAudio2FaceParameters* FaceParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F")
	FName ProviderName = "Default";

	// A2F Provider Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	EA2FProviderType ProviderType = EA2FProviderType::Remote;

	// Remote Provider Settings (API-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Remote Provider", meta = (EditCondition = "ProviderType == EA2FProviderType::Remote"))
	FString RemoteServerURL = "https://api.nvidia.com/v1/ace";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Remote Provider", meta = (EditCondition = "ProviderType == EA2FProviderType::Remote"))
	FString NVIDIAAPIKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Remote Provider", meta = (EditCondition = "ProviderType == EA2FProviderType::Remote"))
	FString NvCFFunctionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Remote Provider", meta = (EditCondition = "ProviderType == EA2FProviderType::Remote"))
	FString NvCFFunctionVersion;

	// Local Provider Settings (GPU-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Local Provider", meta = (EditCondition = "ProviderType == EA2FProviderType::Local"))
	EA2FLocalModel LocalModelType = EA2FLocalModel::Mark;

	// Provider Names
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	FName RemoteProviderName = "RemoteA2F";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	FName LocalProviderName = "LocalA2F";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	FName LocalProviderName_Mark = "LocalA2F-Mark";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	FName LocalProviderName_Claire = "LocalA2F-Claire";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	FName LocalProviderName_James = "LocalA2F-James";

	// Provider state tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F|Provider")
	bool bUseRemoteProvider = true;

private:
	UPROPERTY()
	UAsyncActionAnimateCharacter* CurrentAsyncAction;

	// Lazy loading support
	bool bIsInitialized = false;
	mutable FCriticalSection InitializationCS;

	UFUNCTION()
	void OnAnimationCompleted(bool bSuccess);

	void InitializeDefaultParameters();
	
	// Lazy initialization function
	void EnsureInitialized();

	// Provider management helper methods
	void ConfigureProviderSettings();
	FName GetCurrentProviderName() const;
	FName GetLocalProviderName(EA2FLocalModel ModelType) const;
};