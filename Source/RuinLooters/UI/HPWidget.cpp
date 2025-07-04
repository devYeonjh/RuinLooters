// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HPWidget.h"
#include "Components/ProgressBar.h"

void UHPWidget::CalculateHp(int32 NewCurrentHp, int32 NewMaxHp)
{
    if (!HpBar) return;

    // SetPercent()가 float 타입을 받기 때문에 타입변환
    float Ratio = (float)NewCurrentHp / NewMaxHp;
    HpBar->SetPercent(Ratio);
    UE_LOG(LogTemp, Warning, TEXT("%d / %d"), NewCurrentHp, NewMaxHp);
}

// 적이 죽 위젯 삭제
void UHPWidget::DestroyWidget()
{
    RemoveFromParent();
}



