#include "Animation/ComboInputNotify.h"
#include "Character/RLCharacterBase.h"

void UComboInputNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (ARLCharacterBase* Character = Cast<ARLCharacterBase>(MeshComp->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("[ComboInputNotify] CurrentComboStep: %d, ComboMaxStep: %d, bComboInputBuffered: %s"),
            Character->CurrentComboStep, Character->ComboMaxStep, Character->bComboInputBuffered ? TEXT("true") : TEXT("false"));

        // 마지막 콤보 섹션이면 버퍼만 초기화하고 아무 동작 안 함
        if (Character->CurrentComboStep == Character->ComboMaxStep)
        {
            Character->bComboInputBuffered = false;
            UE_LOG(LogTemp, Warning, TEXT("[ComboInputNotify] Last combo section, buffer cleared."));
            return;
        }

        // 버퍼가 있으면 콤보 진행
        if (Character->bComboInputBuffered)
        {
            Character->CurrentComboStep++;
            Character->PlayComboMontage(Character->CurrentComboStep);
            Character->bComboInputBuffered = false;
            UE_LOG(LogTemp, Warning, TEXT("[ComboInputNotify] Combo advanced to step %d"), Character->CurrentComboStep);
        }
        // 버퍼가 없으면 아무 동작 안 함 (콤보 종료는 OnMontageEnded 등에서 처리)
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[ComboInputNotify] No buffered input, combo not advanced."));
        }
    }
} 