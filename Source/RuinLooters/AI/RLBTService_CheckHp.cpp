// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/RLBTService_CheckHp.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Character/RLCharacterEnemyDragon.h"

URLBTService_CheckHp::URLBTService_CheckHp()
{
    // 서비스 이름 설정
    NodeName = TEXT("Check Dragon HP");
    
    // 틱 간격 설정 (0.5초마다 체크)
    Interval = 0.5f;
    
    // 블랙보드 키 필터 설정
    bIsHpLowKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(URLBTService_CheckHp, bIsHpLowKey));
}

void URLBTService_CheckHp::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    // 블랙보드 컴포넌트 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }
    
    // AI 컨트롤러 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return;
    }
    
    // 드래곤 캐릭터 가져오기
    ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
    if (!Dragon)
    {
        return;
    }
    
    // 현재 체력과 최대 체력 가져오기
    float CurrentHp = Dragon->GetCurrentHp();
    float MaxHp = Dragon->GetMaxHp();
    
    // 체력이 0 이하이거나 최대 체력이 0 이하인 경우 예외 처리
    if (MaxHp <= 0)
    {
        return;
    }
    
    // 체력 비율 계산
    float HpRatio = CurrentHp / MaxHp;
    
    // 체력이 임계값 이하인지 확인
    bool bIsHpLow = HpRatio <= HpThreshold;
    
    // 블랙보드에 결과 설정
    BlackboardComp->SetValueAsBool(bIsHpLowKey.SelectedKeyName, bIsHpLow);
    
    // 디버그 로그 (선택적)
    UE_LOG(LogTemp, Log, TEXT("Dragon HP Check: %.1f/%.1f (%.2f%%), IsHpLow: %s"), 
           CurrentHp, MaxHp, HpRatio * 100.0f, bIsHpLow ? TEXT("True") : TEXT("False"));
} 