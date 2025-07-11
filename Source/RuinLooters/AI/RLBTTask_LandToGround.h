// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RLBTTask_LandToGround.generated.h"

/**
 * 캐릭터의 MovementMode를 Walk로 바꾸고 땅까지 내려오는 태스크
 * 중력의 영향을 받지 않는 경우 강제로 아래로 이동시킵니다.
 */
UCLASS()
class RUINLOOTERS_API URLBTTask_LandToGround : public UBTTaskNode
{
	GENERATED_BODY()

public:
	URLBTTask_LandToGround();

protected:
	// 땅으로부터의 허용 거리 (이 거리 내에 도달하면 성공)
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptableGroundDistance = 210.0f;

	// 강제 하강 속도 (중력이 작동하지 않을 때 사용)
	UPROPERTY(EditAnywhere, Category = "AI")
	float ForceDownSpeed = 500.0f;

	// 라인 트레이스 거리 (땅 감지용)
	UPROPERTY(EditAnywhere, Category = "AI")
	float GroundCheckDistance = 2000.0f;

	// 최대 대기 시간 (초) - 이 시간이 지나면 실패
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxWaitTime = 10.0f;

	// 중력 확인 시간 (초) - 이 시간 동안 중력이 작동하지 않으면 강제 하강
	UPROPERTY(EditAnywhere, Category = "AI")
	float GravityCheckTime = 1.0f;

	// 착지 후 실행할 새로운 BT 에셋 (선택사항)
	UPROPERTY(EditAnywhere, Category = "AI|Behavior Tree Switch")
	class UBehaviorTree* NewBehaviorTreeOnLanding = nullptr;

	// 착지 후 BT 교체 기능 활성화 여부
	UPROPERTY(EditAnywhere, Category = "AI|Behavior Tree Switch")
	bool bSwitchBehaviorTreeOnLanding = false;

	// Task 시작
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Task 진행 상황 업데이트
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 경과 시간 추적
	float ElapsedTime;

	// 중력 확인 시간 추적
	float GravityCheckElapsed;

	// 시작 위치 캐싱
	FVector StartLocation;

	// 이전 위치 (중력 작동 여부 확인용)
	FVector PreviousLocation;

	// 강제 하강 모드 여부
	bool bForceDownMode;

	// 땅까지의 거리를 계산하는 함수
	float GetDistanceToGround(class ACharacter* Character);

	// 착지 후 BT 교체 처리 함수
	void HandleBehaviorTreeSwitch(UBehaviorTreeComponent& OwnerComp);
}; 