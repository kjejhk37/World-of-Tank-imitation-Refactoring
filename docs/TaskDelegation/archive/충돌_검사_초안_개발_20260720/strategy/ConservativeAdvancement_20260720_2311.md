../brainstorming/충돌감지_알고리즘_20260720_2119.md

# Strategy: Conservative Advancement (CCD) (로드맵 9번)

## 적용 방식

연속 충돌 감지(CCD). 두 Geometry가 각자 선속도(velocity)를 가지고 `deltaTime` 동안 이동할 때, 그 사이에 충돌이 발생하는지와 충돌 시각(Time of Impact, TOI)을 구한다. 이산(discrete) 알고리즘(GJK/EPA 등)은 프레임 시작/끝 시점만 검사해 고속 이동체(포탄)가 얇은 도형을 통과(터널링)해버릴 수 있는데, CCD는 그 사이 시간을 반복적으로 좁혀가며 실제 접촉 시점을 찾는다.

`GetBounds()`만 사용, GJK/EPA 등 다른 알고리즘 모듈을 참조하지 않는다(독립성).

### 파일 구성 (`src/collision/conservative_advancement/`)

- `ConservativeAdvancement.h`/`.cpp` — `struct ToiResult { bool collided; float timeOfImpact; }; ToiResult ConservativeAdvancement(const Geometry& a, const Vec3& velocityA, const Geometry& b, const Vec3& velocityB, float deltaTime);`

### 알고리즘 개요 (경계구 기반 단순화)

정확한 알고리즘은 매 반복마다 실제 도형의 지지 함수 기반 최단 거리(GJK 스타일)를 구해야 하지만, 이를 위해서는 "이동 중인 도형 사이의 최단 거리" 쿼리(Johnson's sub-distance algorithm 등)가 별도로 필요하고 이번 사이클 범위를 크게 넘어선다. 대신 각 도형의 `GetBounds()`가 주는 경계 구(bounding sphere: 중심 = AABB 중심, 반지름 = AABB의 half-diagonal 길이)로 대체한다 — 실제 도형은 항상 이 경계 구 안에 있으므로, 경계 구 사이 거리는 실제 두 도형 사이 최단 거리의 **항상 안전한(보수적인) 하한**이다(즉 "경계 구가 아직 안 닿았으면 실제 도형도 아직 안 닿았다"가 항상 참 — Conservative Advancement라는 이름의 "보수적"이 의미하는 안전성은 그대로 유지됨).

1. `t=0`에서 시작. 각 도형의 경계 구 중심 `centerA(t) = centerA0 + velocityA*t`, 반지름은 이동해도 불변.
2. 현재 `t`에서 경계 구 사이 거리 `distance = |centerB(t)-centerA(t)| - (radiusA+radiusB)` 계산.
3. `distance`가 허용오차 이하면 충돌(TOI = 현재 t)로 판정.
4. 그렇지 않으면 두 중심을 잇는 방향으로의 접근 속도(`closingSpeed`)를 계산 — 0 이하(서로 멀어지거나 평행)면 이번 프레임 안에 충돌 없음으로 판정.
5. 안전 전진 시간 `distance / closingSpeed`만큼 `t`를 전진, `t >= deltaTime`이면 충돌 없음으로 판정, 아니면 2번부터 반복(반복 상한 있음).

### 테스트 전략

고속으로 서로를 향해 다가오는 두 구(경계 구 자체가 정확한 케이스라 해석적 TOI와 비교 가능) + 서로 멀어지는 경우(충돌 없음) + 터널링 시나리오(한 프레임 안에서 discrete 검사라면 놓칠 만큼 빠른 이동, deltaTime 안에 실제로 충돌이 감지되는지)를 검증한다.

---

## Checklist

- [x] `src/collision/conservative_advancement/ConservativeAdvancement.h`/`.cpp` 구현
- [x] `CMakeLists.txt`에 `.cpp` + 테스트 파일 추가
- [x] `tests/ConservativeAdvancementTest.cpp` — 충돌/비충돌/터널링 방지 케이스
- [x] 빌드 + 테스트 통과 확인 (개별 코드 리뷰 생략 — 로드맵 10번까지 완료 후 일괄 진행)

## 승인/검증 기준

- [x] 정면으로 다가와 충돌하는 두 도형의 TOI가 해석적 값(경계 구 반지름 합 기준)과 1e-2 오차 이내로 일치.
- [x] 서로 멀어지거나(즉시 closingSpeed<=0) 스쳐 지나가는 경우(최소 거리가 경계 구 합보다 큼) 충돌 없음으로 정확히 판정.
- [x] 고속 이동으로 인한 터널링 시나리오(한 프레임 동안 x=0→1000, 중간 x=50의 얇은 벽)에서도 충돌이 정상 검출됨(TOI가 (0, deltaTime) 사이).
- [x] GJK/EPA 등 다른 알고리즘 모듈을 include하지 않음(독립성) — `geometry/Geometry.h`, `math/MathConstants.h`만 include.
