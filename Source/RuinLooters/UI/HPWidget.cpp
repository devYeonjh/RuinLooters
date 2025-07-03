// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HPWidget.h"
#include "Components/ProgressBar.h"

void UHPWidget::CalculateHp(int32 NewCurrentHp, int32 NewMaxHp)
{
    if (!HpBar) return;

    // SetPercent()가 float 타입만 받기 때문에 형변환
    float Ratio = (float)NewCurrentHp / NewMaxHp;
    HpBar->SetPercent(Ratio);
    UE_LOG(LogTemp, Warning, TEXT("%d / %d"), NewCurrentHp, NewMaxHp);
}

// 사망 시 위젯 제거
void UHPWidget::DestroyWidget()
{
    RemoveFromParent();
}
