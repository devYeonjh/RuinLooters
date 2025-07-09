#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "InvincibleRollNotifyState.generated.h"

<<<<<<<< HEAD:Source/RuinLooters/Animation/InvincibleRollNotifyState.h
UCLASS()
class RUINLOOTERS_API UInvincibleRollNotifyState : public UAnimNotifyState
{
    GENERATED_BODY()
public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
}; 
========
// ... 기존 코드 삭제(AnimNotifyState가 아니라 AnimNotify로 분리 예정) ... 
>>>>>>>> feature/comboattack:Source/RuinLooters/Animation/ComboInputWindowNotifyState.h
