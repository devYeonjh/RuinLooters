#include "InvincibleRollNotifyState.h"
#include "Character/RLCharacterPlayer.h"

void UInvincibleRollNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(MeshComp->GetOwner()))
    {
        //Player->StartInvincible();
    }
}

void UInvincibleRollNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(MeshComp->GetOwner()))
    {
        //Player->EndInvincible();
    }
} 