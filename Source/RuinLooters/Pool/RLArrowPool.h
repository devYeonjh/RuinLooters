// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RLArrowPool.generated.h"

class ARLArrow;

/**
 * 화살 풀링 관리 클래스
 */
UCLASS()
class RUINLOOTERS_API URLArrowPool : public UObject
{
	GENERATED_BODY()

public:
	URLArrowPool();

	/**
	 * 화살 풀 초기화
	 * @param World 월드 컨텍스트
	 * @param ArrowClass 화살 클래스
	 * @param InitialPoolSize 초기 풀 크기
	 */
	UFUNCTION()
	void InitializeArrowPool(UWorld* World, TSubclassOf<ARLArrow> ArrowClass, int32 InitialPoolSize = 10);

	/**
	 * 풀에서 화살 가져오기
	 * @return 사용 가능한 화살 객체
	 */
	UFUNCTION()
	ARLArrow* GetArrowFromPool();

	/**
	 * 화살을 풀에 반환
	 * @param Arrow 반환할 화살 객체
	 */
	UFUNCTION()
	void ReturnArrowToPool(ARLArrow* Arrow);

	/**
	 * 풀 정리 (게임 종료 시)
	 */
	UFUNCTION()
	void ClearPool();

private:
	// 비활성 화살 풀
	UPROPERTY()
	TArray<ARLArrow*> ArrowPool;

	// 활성 화살 목록
	UPROPERTY()
	TArray<ARLArrow*> ActiveArrows;

	// 화살 클래스 레퍼런스
	UPROPERTY()
	TSubclassOf<ARLArrow> ArrowClass;

	// 월드 컨텍스트
	UPROPERTY()
	UWorld* World;
}; 