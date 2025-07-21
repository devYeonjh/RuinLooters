#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ACETypes.h"
#include "Audio2FaceParameters.h"
#include "AsyncActionAnimateCharacter.h"
#include "RLA2FComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnA2FAnimationCompleted, bool, bSuccess);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F")
	FAudio2FaceEmotion EmotionParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F")
	UAudio2FaceParameters* FaceParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RLA2F")
	FName ProviderName = "Default";

	UPROPERTY(BlueprintAssignable, Category = "RLA2F")
	FOnA2FAnimationCompleted OnA2FAnimationCompleted;

	UFUNCTION(BlueprintCallable, Category = "RLA2F")
	bool IsAnimationInProgress() const { return CurrentAsyncAction != nullptr; }

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
};