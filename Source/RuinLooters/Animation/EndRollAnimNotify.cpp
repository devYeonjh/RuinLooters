#include "EndRollAnimNotify.h"
#include "Character/RLCharacterPlayer.h"

void UEndRollAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (ARLCharacterPlayer* Player = Cast<ARLCharacterPlayer>(MeshComp->GetOwner()))
    {
        Player->EndRoll();
    }
} 