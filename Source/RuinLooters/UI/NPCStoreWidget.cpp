// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NPCStoreWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLCharacterPlayer.h"
#include "Character/RLNPC.h"

void UNPCStoreWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (BuyButton)
    {
        BuyButton->OnClicked.AddDynamic(this, &UNPCStoreWidget::BuyItem);
    }
    if (DeleteButton)
    {
        DeleteButton->OnClicked.AddDynamic(this, &UNPCStoreWidget::DeleteWidget);
    }

    Player = Cast<ARLCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void UNPCStoreWidget::BuyItem()
{
    OnBuyPressed.Broadcast();
}

void UNPCStoreWidget::DeleteWidget()
{
    Player->GetInteractNPC()->RemoveWidget();
}



