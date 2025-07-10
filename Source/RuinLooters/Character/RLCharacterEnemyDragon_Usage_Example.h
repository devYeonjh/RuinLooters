// 투사체 풀링 시스템 사용 예제
// 이 파일은 드래곤 캐릭터에서 투사체 풀링을 사용하는 방법을 보여줍니다.

#pragma once

/*
===========================================
투사체 풀링 시스템 사용 가이드
===========================================

1. 드래곤 캐릭터 설정:
   - 블루프린트에서 Dragon Character의 ProjectileClass를 설정
   - BP_DragonBreathProjectile 또는 적절한 투사체 클래스 할당

2. 브레스 공격 호출:
   - AI 컨트롤러나 비헤이비어 트리에서 BreathAttack() 함수 호출
   - 또는 플레이어 입력에 따라 브레스 공격 트리거

3. 투사체 풀 관리:
   - 투사체 풀은 자동으로 초기화되고 관리됩니다
   - 투사체는 생존 시간 후 자동으로 풀에 반환됩니다

===========================================
블루프린트 설정 예제
===========================================

1. BP_DragonEnemy 블루프린트에서:
   - Projectile Class: BP_DragonBreathProjectile 설정
   - Breath Montage: 브레스 공격 애니메이션 몽타주 설정
   - Breath Damage: 브레스 공격 데미지 설정 (기본값: 75)
   - Breath Range: 브레스 공격 범위 설정 (기본값: 1000)

2. AI 컨트롤러에서:
   - 특정 조건에서 Dragon->BreathAttack() 호출
   - 또는 비헤이비어 트리에서 Custom Task로 브레스 공격 실행

===========================================
코드 사용 예제
===========================================

// AI 컨트롤러에서 브레스 공격 호출
void AYourAIController::PerformBreathAttack()
{
    ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(GetPawn());
    if (Dragon)
    {
        Dragon->BreathAttack();
    }
}

// 비헤이비어 트리 Task에서 브레스 공격 호출
EBTNodeResult::Type UYourBreathAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        ARLCharacterEnemyDragon* Dragon = Cast<ARLCharacterEnemyDragon>(AIController->GetPawn());
        if (Dragon)
        {
            Dragon->BreathAttack();
            return EBTNodeResult::Succeeded;
        }
    }
    return EBTNodeResult::Failed;
}

// 플레이어 입력에서 브레스 공격 호출 (테스트용)
void AYourPlayerController::TestDragonBreath()
{
    // 월드에서 드래곤 찾기
    for (TActorIterator<ARLCharacterEnemyDragon> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ARLCharacterEnemyDragon* Dragon = *ActorItr;
        if (Dragon)
        {
            Dragon->BreathAttack();
            break;
        }
    }
}

===========================================
투사체 커스터마이징
===========================================

투사체 설정을 변경하려면:

1. RLDragonBreathProjectile.cpp에서:
   - 속도: InitialSpeed, MaxSpeed 값 조정
   - 데미지: Damage 기본값 조정
   - 생존 시간: LifeTime 값 조정
   - 콜리전: SphereCollision 설정 조정

2. 파티클 효과:
   - 블루프린트에서 Particle System 컴포넌트에 적절한 파티클 시스템 할당
   - 화염 효과, 연기 효과 등 원하는 비주얼 이펙트 추가

3. 사운드 효과:
   - 투사체 발사 사운드
   - 투사체 충돌 사운드
   - 폭발 사운드 등

===========================================
성능 최적화 팁
===========================================

1. 풀 크기 조정:
   - 너무 작으면 자주 새로운 객체 생성
   - 너무 크면 메모리 낭비
   - 일반적으로 10-20개 정도가 적당

2. 투사체 생존 시간:
   - 맵 크기에 따라 적절히 조정
   - 너무 길면 메모리 사용량 증가
   - 너무 짧으면 투사체가 목표에 도달하지 못함

3. 콜리전 최적화:
   - 단순한 콜리전 형태 사용 (구체, 캡슐)
   - 복잡한 메시 콜리전 사용 금지
   - 적절한 콜리전 채널 설정

===========================================
*/ 