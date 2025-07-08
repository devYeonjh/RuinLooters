// 플레이어 투사체 스킬 시스템 사용 예제
// 이 파일은 플레이어가 투사체 스킬을 사용하는 방법을 보여줍니다.

#pragma once

/*
===========================================
플레이어 투사체 스킬 시스템 사용 가이드
===========================================

## 구현된 기능들:

### 1. 플레이어 전용 투사체 (RLPlayerProjectile)
- **빠른 속도**: 3000 유닛/초 (드래곤 1500 vs 플레이어 3000)
- **관통 능력**: 최대 3개의 적을 관통하여 데미지 적용
- **작은 콜리전**: 15 유닛 반지름 (드래곤 25 vs 플레이어 15)
- **짧은 생존 시간**: 3초 (드래곤 5초 vs 플레이어 3초)
- **중간 데미지**: 35 데미지 (드래곤 75 vs 플레이어 35)

### 2. 플레이어 투사체 스킬 시스템
- **쿨다운 시스템**: 3초 쿨다운으로 연속 사용 방지
- **투사체 풀링**: 15개 투사체를 미리 생성하여 성능 최적화
- **입력 바인딩**: ProjectileSkillAction으로 바인딩
- **자동 반환**: 3초 후 자동으로 풀에 반환

### 3. 드래곤 vs 플레이어 투사체 비교

| 특성 | 드래곤 브레스 | 플레이어 투사체 |
|------|---------------|----------------|
| 데미지 | 75 | 35 |
| 속도 | 1500 | 3000 |
| 콜리전 크기 | 25 | 15 |
| 생존 시간 | 5초 | 3초 |
| 특수 능력 | 화염 효과 | 관통 능력 (3개) |
| 풀 크기 | 10개 | 15개 |
| 쿨다운 | 없음 | 3초 |

===========================================
블루프린트 설정 가이드
===========================================

### 1. BP_PlayerCharacter 블루프린트 설정:
```
- Player Projectile Class: BP_PlayerProjectile 설정
- Projectile Damage: 투사체 데미지 설정 (기본값: 40)
- Projectile Speed: 투사체 속도 설정 (기본값: 3000)
- Projectile Skill Cooldown: 스킬 쿨다운 시간 설정 (기본값: 3초)
```

### 2. 입력 매핑 설정:
```
- Input Actions에 ProjectileSkillAction 추가
- 키 바인딩: 원하는 키 (예: F키, 마우스 우클릭 등)
- Trigger Event: Triggered
```

### 3. 투사체 블루프린트 설정:
```
- BP_PlayerProjectile 생성 (RLPlayerProjectile 기반)
- Particle System: 원하는 파티클 이펙트 설정
- Can Pierce Enemies: true (관통 활성화)
- Max Pierce Count: 3 (최대 관통 개수)
```

===========================================
코드 사용 예제
===========================================

### C++에서 투사체 스킬 사용:
```cpp
// 플레이어 캐릭터에서 투사체 스킬 사용
void AYourGameMode::TestPlayerProjectileSkill()
{
    ARLCharacterPlayer* Player = GetWorld()->GetFirstPlayerController()->GetPawn<ARLCharacterPlayer>();
    if (Player)
    {
        Player->UseProjectileSkill();
    }
}
```

### 블루프린트에서 투사체 스킬 사용:
```
1. Player Character Reference 가져오기
2. Use Projectile Skill 함수 호출
3. 또는 Fire Player Projectile 함수 직접 호출
```

### AI에서 플레이어 투사체 회피:
```cpp
// AI 컨트롤러에서 플레이어 투사체 감지 및 회피
void AYourAIController::CheckForPlayerProjectiles()
{
    TArray<AActor*> FoundProjectiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARLPlayerProjectile::StaticClass(), FoundProjectiles);
    
    for (AActor* ProjectileActor : FoundProjectiles)
    {
        ARLPlayerProjectile* Projectile = Cast<ARLPlayerProjectile>(ProjectileActor);
        if (Projectile && Projectile->GetOwner() != GetPawn())
        {
            // 투사체 회피 로직 구현
            AvoidProjectile(Projectile);
        }
    }
}
```

===========================================
성능 최적화 설정
===========================================

### 1. 투사체 풀 크기 조정:
```cpp
// 플레이어 캐릭터 생성자에서
ProjectilePoolSize = 15; // 일반적으로 10-20개가 적당

// 많은 적이 있는 레벨: 20개
// 적은 적이 있는 레벨: 10개
```

### 2. 투사체 설정 최적화:
```cpp
// 빠른 전투용 설정
ProjectileDamage = 30;
ProjectileSpeed = 3500.0f;
ProjectileSkillCooldown = 2;

// 균형잡힌 설정 (기본값)
ProjectileDamage = 40;
ProjectileSpeed = 3000.0f;
ProjectileSkillCooldown = 3;

// 강력한 설정
ProjectileDamage = 50;
ProjectileSpeed = 2500.0f;
ProjectileSkillCooldown = 4;
```

### 3. 관통 시스템 조정:
```cpp
// 관통 특화 설정
MaxPierceCount = 5;
bCanPierceEnemies = true;
Damage = 25; // 관통이 많으면 데미지 낮춤

// 단일 대상 특화 설정
MaxPierceCount = 1;
bCanPierceEnemies = false;
Damage = 60; // 관통이 없으면 데미지 높임
```

===========================================
사용 팁
===========================================

### 1. 전투 전략:
- 여러 적이 일렬로 있을 때 관통 효과 활용
- 강한 적에게는 근접해서 확실히 명중
- 쿨다운을 고려하여 적절한 타이밍에 사용

### 2. 레벨 디자인 고려사항:
- 좁은 통로에서 관통 효과 극대화
- 넓은 공간에서는 정확한 조준 필요
- 높은 플랫폼에서 아래로 발사하여 여러 적 타격

### 3. 업그레이드 시스템 아이디어:
- 데미지 증가 아이템
- 관통 개수 증가 아이템
- 쿨다운 감소 아이템
- 투사체 속도 증가 아이템
- 투사체 크기 증가 아이템

===========================================
*/ 