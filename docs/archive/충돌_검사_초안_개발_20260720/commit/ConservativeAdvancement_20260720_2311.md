../strategy/ConservativeAdvancement_20260720_2311.md

# Commit: Conservative Advancement (CCD)

## 구현 결과

- `src/collision/conservative_advancement/ConservativeAdvancement.h`/`.cpp` — `ToiResult ConservativeAdvancement(a, velocityA, b, velocityB, deltaTime)`: 각 도형의 `GetBounds()` 경계 구(중심=AABB 중심, 반지름=half-diagonal)로 거리를 근사, 접근 속도로 안전 전진 시간을 반복 계산해 TOI를 좁힘.
- 경계 구는 실제 도형을 항상 포함하므로 "경계 구가 안 닿았으면 실제 도형도 안 닿았다"가 항상 성립 — 보수적(안전) 근사.
- `CMakeLists.txt`: `app_lib`/`tests`에 각각 `.cpp` 추가.
- `tests/ConservativeAdvancementTest.cpp`: 정면 충돌(해석적 TOI 비교), 이탈(즉시 비충돌), 스쳐 지나감(최소 거리가 경계 구 합보다 큼), 고속 이동 터널링 방지(얇은 벽을 관통하듯 빠르게 이동해도 충돌 정상 검출) 총 4개.
- GJK/EPA 등 다른 알고리즘 모듈을 전혀 참조하지 않음(독립성 확인).

## 현재 상태에서 확인된 이슈

- [RESOLVED] `Win32WindowTest.PumpMessagesReturnsTrueWhenNoQuitPosted` 1건 실패 — 이전 사이클들에서도 동일하게 보고된 기존 이슈, 이번 사이클과 무관.
  → resolved: `tests/Win32WindowTest.cpp`에 픽스처 추가, SetUp()에서 잔여 WM_QUIT 드레인. 227/227 테스트 통과 확인 (2026-07-22).
- [DEFERRED] (범위 밖, 기록용) 이번 구현은 실제 지지 함수 기반 최단 거리 대신 경계 구로 근사한다 — 늘어진(가늘고 긴) 도형일수록 경계 구가 실제 형태보다 훨씬 헐거워 TOI가 실제보다 이르게(더 보수적으로) 나올 수 있다. 정확한 지지 함수 기반 CCD는 별도의 최단 거리 서브 알고리즘이 필요해 다음 사이클 후보로 기록만 함.

## 체크리스트/검증 상태

전략 문서(`../strategy/ConservativeAdvancement_20260720_2311.md`)의 체크리스트 4개 전부 `[x]`, 승인/검증 기준 4개 전부 충족.

## 다음 단계

코드 리뷰는 생략(로드맵 10번까지 완료 후 일괄 진행), 로드맵 10번(커스텀 FCL 통합 파이프라인 — 마지막 항목, 앞선 모든 모듈에 의존)으로 진행.
