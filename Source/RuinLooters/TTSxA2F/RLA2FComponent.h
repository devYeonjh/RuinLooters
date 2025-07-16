#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ACETypes.h"
#include "Audio2FaceParameters.h"
#include "AsyncActionAnimateCharacter.h"
#include "RLA2FComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RUINLOOTERS_API URLA2FComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URLA2FComponent();

protected:
	virtual void BeginPlay() override;

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

private:
	UPROPERTY()
	UAsyncActionAnimateCharacter* CurrentAsyncAction;

	UFUNCTION()
	void OnAnimationCompleted(bool bSuccess);

	void InitializeDefaultParameters();
};