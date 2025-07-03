#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RLNPCBuyInterface.generated.h"

UINTERFACE(MinimalAPI)
class URLNPCBuyInterface : public UInterface
{
    GENERATED_BODY()
};

// 반드시 IInterface 를 상속하도록!
class RUINLOOTERS_API IRLNPCBuyInterface : public IInterface
{
    GENERATED_BODY()

public:
    /// 상점 구매 처리를 구현할 함수 (순수 가상)
    virtual void HandleStoreBuy() = 0;
};