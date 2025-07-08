// 통합 투사체 시스템 가이드
// RLProjectile 클래스로 드래곤과 플레이어 투사체를 통합했습니다.

#pragma once

/*
===========================================
통합 투사체 시스템 (RLProjectile)
===========================================

## 주요 변경사항:

### 1. 클래스 통합
- **이전**: RLDragonBreathProjectile + RLPlayerProjectile (두 개의 별도 클래스)
- **현재**: RLProjectile (하나의 범용 클래스)

### 2. 설정 기반 시스템
- **FProjectileSettings 구조체**: 모든 투사체 설정을 한 곳에서 관리
- **EProjectileType 열거형**: PlayerProjectile, DragonBreath, Generic 타입 구분
- **프리셋 함수**: SetupAsPlayerProjectile(), SetupAsDragonBreath()

### 3. 코드 중복 제거
- 기본 투사체 기능 통합 (콜리전, 이동, 파티클)
- 오브젝트 풀링 시스템 통합
- 타이머 관리 통합

===========================================
투사체 설정 비교표
===========================================

| 설정 항목 | 플레이어 투사체 | 드래곤 브레스 |
|-----------|----------------|---------------|
| **타입** | PlayerProjectile | DragonBreath |
| **데미지** | 40 | 75 |
| **속도** | 3000 | 1500 |
| **생존시간** | 3초 | 5초 |
| **콜리전 반지름** | 15 | 25 |
| **관통 가능** | true | false |
| **최대 관통** | 3개 | 1개 |
| **타겟** | 적만 | 플레이어만 |

===========================================
사용법 - 플레이어 투사체
===========================================

### C++ 코드에서:
```cpp
// 방법 1: 프리셋 사용
ARLProjectile* Projectile = GetProjectileFromPool();
Projectile->SetupAsPlayerProjectile();
Projectile->InitializeProjectileSimple(StartLocation, Direction, 40, 3000.0f);

// 방법 2: 직접 설정
FProjectileSettings PlayerSettings;
PlayerSettings.ProjectileType = EProjectileType::PlayerProjectile;
PlayerSettings.Damage = 40;
PlayerSettings.Speed = 3000.0f;
PlayerSettings.LifeTime = 3.0f;
PlayerSettings.CollisionRadius = 15.0f;
PlayerSettings.bCanPierceEnemies = true;
PlayerSettings.MaxPierceCount = 3;

Projectile->InitializeProjectile(StartLocation, Direction, PlayerSettings);
```

### 블루프린트에서:
```
1. ARLProjectile 클래스를 상속받아 BP_PlayerProjectile 생성
2. Setup As Player Projectile 함수 호출
3. Initialize Projectile 함수로 발사
```

===========================================
사용법 - 드래곤 브레스
===========================================

### C++ 코드에서:
```cpp
// 방법 1: 프리셋 사용
ARLProjectile* Projectile = GetProjectileFromPool();
Projectile->SetupAsDragonBreath();
Projectile->InitializeProjectileSimple(StartLocation, Direction, 75, 1500.0f);

// 방법 2: 직접 설정
FProjectileSettings DragonSettings;
DragonSettings.ProjectileType = EProjectileType::DragonBreath;
DragonSettings.Damage = 75;
DragonSettings.Speed = 1500.0f;
DragonSettings.LifeTime = 5.0f;
DragonSettings.CollisionRadius = 25.0f;
DragonSettings.bCanPierceEnemies = false;
DragonSettings.MaxPierceCount = 1;

Projectile->InitializeProjectile(StartLocation, Direction, DragonSettings);
```

===========================================
오브젝트 풀링 업데이트
===========================================

### 기존 코드:
```cpp
// 플레이어용
URLProjectilePool* PlayerPool;  // ARLPlayerProjectile 전용
URLProjectilePool* DragonPool;  // ARLDragonBreathProjectile 전용
```

### 새로운 코드:
```cpp
// 통합 풀 (둘 다 ARLProjectile 사용)
URLProjectilePool* PlayerPool;  // ARLProjectile 사용
URLProjectilePool* DragonPool;  // ARLProjectile 사용

// 또는 하나의 풀로 통합 가능
URLProjectilePool* UnifiedPool; // 모든 투사체 공용
```

===========================================
마이그레이션 가이드
===========================================

### 1. 헤더 파일 변경:
```cpp
// 이전
#include "RLDragonBreathProjectile.h"
#include "RLPlayerProjectile.h"

// 현재
#include "RLProjectile.h"
```

### 2. 클래스 레퍼런스 변경:
```cpp
// 이전
TSubclassOf<ARLDragonBreathProjectile> DragonProjectileClass;
TSubclassOf<ARLPlayerProjectile> PlayerProjectileClass;

// 현재
TSubclassOf<ARLProjectile> ProjectileClass;  // 둘 다 동일
```

### 3. 투사체 생성 변경:
```cpp
// 이전
ARLDragonBreathProjectile* DragonProjectile = Pool->GetProjectile();
ARLPlayerProjectile* PlayerProjectile = Pool->GetProjectile();

// 현재
ARLProjectile* Projectile = Cast<ARLProjectile>(Pool->GetProjectile());
```

### 4. 초기화 방법 변경:
```cpp
// 이전
DragonProjectile->InitializeProjectile(Loc, Dir, Damage, Speed);
PlayerProjectile->InitializeProjectile(Loc, Dir, Damage, Speed);

// 현재 - 방법 1 (프리셋)
Projectile->SetupAsDragonBreath();
Projectile->InitializeProjectileSimple(Loc, Dir, Damage, Speed);

// 현재 - 방법 2 (설정 구조체)
FProjectileSettings Settings = CreateDragonSettings();
Projectile->InitializeProjectile(Loc, Dir, Settings);
```

===========================================
커스텀 투사체 생성
===========================================

### 새로운 투사체 타입 추가:
```cpp
// 1. EProjectileType에 새 타입 추가
enum class EProjectileType : uint8
{
    PlayerProjectile,
    DragonBreath,
    MagicMissile,    // 새 타입
    FireArrow,       // 새 타입
    Generic
};

// 2. 프리셋 함수 추가
void ARLProjectile::SetupAsMagicMissile()
{
    FProjectileSettings Settings;
    Settings.ProjectileType = EProjectileType::MagicMissile;
    Settings.Damage = 60;
    Settings.Speed = 2500.0f;
    Settings.LifeTime = 4.0f;
    Settings.CollisionRadius = 20.0f;
    Settings.bCanPierceEnemies = true;
    Settings.MaxPierceCount = 2;
    
    ApplyProjectileSettings(Settings);
}

// 3. 충돌 처리에 새 타입 추가 (필요시)
```

===========================================
성능 이점
===========================================

### 1. 코드 중복 제거:
- **이전**: 2개 클래스 × 각각 ~200줄 = 400줄
- **현재**: 1개 클래스 × 300줄 = 300줄 (25% 감소)

### 2. 메모리 효율성:
- 하나의 풀로 통합 가능
- 런타임에 투사체 타입 변경 가능
- 더 적은 클래스 로딩

### 3. 유지보수성:
- 버그 수정이 모든 투사체에 적용
- 새 기능 추가가 간단
- 설정 변경이 용이

===========================================
문제 해결
===========================================

### Q: 기존 블루프린트가 작동하지 않음
A: 블루프린트에서 클래스 레퍼런스를 ARLProjectile로 변경

### Q: 투사체가 잘못된 타겟을 공격함
A: SetupAsPlayerProjectile() 또는 SetupAsDragonBreath() 호출 확인

### Q: 관통이 작동하지 않음
A: FProjectileSettings에서 bCanPierceEnemies = true 설정 확인

### Q: 풀에서 가져온 투사체가 이상함
A: InitializeProjectile 호출 전에 반드시 Setup 함수 호출

===========================================
*/ 