#include "Animation/ComboInputWindowNotifyState.h"
#include "Character/RLCharacterBase.h"

void UComboInputWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (ARLCharacterBase* Character = Cast<ARLCharacterBase>(MeshComp->GetOwner()))
    {
        Character->bCanNextCombo = true; // 입력 가능 구간 시작
    }
}

void UComboInputWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (ARLCharacterBase* Character = Cast<ARLCharacterBase>(MeshComp->GetOwner()))
    {
        if (Character->bComboInput && Character->CurrentComboStep < Character->ComboMaxStep)
        {
            Character->CurrentComboStep++;
            Character->PlayComboMontage(Character->CurrentComboStep);
        }
        else
        {
            Character->CurrentComboStep = 0;
            Character->bIsCanAttack = true;
        }
        Character->bCanNextCombo = false;
        Character->bComboInput = false;
    }
} 