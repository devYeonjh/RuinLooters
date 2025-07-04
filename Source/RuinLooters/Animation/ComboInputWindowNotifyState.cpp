#include "Animation/ComboInputWindowNotifyState.h"
#include "Character/RGCharacterBase.h"

void UComboInputWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (ARGCharacterBase* Character = Cast<ARGCharacterBase>(MeshComp->GetOwner()))
    {
        Character->bCanNextCombo = true; // 입력 가능 구간 시작
    }
}

void UComboInputWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (ARGCharacterBase* Character = Cast<ARGCharacterBase>(MeshComp->GetOwner()))
    {
        if (Character->bComboInput && Character->CurrentComboStep < Character->ComboMaxStep)
        {
            Character->CurrentComboStep++;
            Character->PlayComboMontage(Character->CurrentComboStep);
        }
        else
        {
            Character->CurrentComboStep = 0; // 콤보 종료
            Character->bIsCanAttack = true;  // 다시 공격 가능
        }
        Character->bCanNextCombo = false;
        Character->bComboInput = false;
    }
} 