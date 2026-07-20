../brainstorming/충돌감지_알고리즘_20260720_2119.md

# Strategy: Sweep and Prune (로드맵 3번)

## 적용 방식

broad-phase 알고리즘. 여러 Geometry 중 실제 겹칠 가능성이 있는 후보 쌍(index 쌍)을 추려낸다. `GetBounds()` 자유 함수(Geometry.h)만 사용 — GJK/EPA/BVH 등 다른 알고리즘 모듈을 전혀 참조하지 않는다(독립성 요구사항).

### 파일 구성 (`src/collision/sweep_and_prune/`)

- `SweepAndPrune.h`/`.cpp` — `std::vector<std::pair<size_t, size_t>> FindOverlappingPairs(const std::vector<Geometry>& objects)`.

### 알고리즘 개요

1. 각 객체의 `GetBounds()`를 계산해 (원본 index, AABB) 쌍의 목록을 만든다.
2. 이 목록을 `bounds.min.x` 기준으로 오름차순 정렬한다(단일 축 스윕 — 축 선택 최적화(분산이 가장 큰 축 선택 등)는 이번 사이클 범위 밖, 항상 x축 고정).
3. 정렬된 순서로 i를 순회하며, i 이후 j에 대해 `entries[j].bounds.min.x > entries[i].bounds.max.x`이면 그 이후 j는 절대 겹칠 수 없으므로 즉시 break(정렬되어 있으므로 안전).
4. break 전까지의 j에 대해 `geometry/Intersections.h`의 기존 `Intersects(AABB, AABB)`로 3축 전체 겹침을 확인, 겹치면 (원본 index 쌍)을 결과에 추가.

기존 `Intersections.h`의 AABB-AABB 겹침 검사를 재사용해 SRP를 지킨다(SAP은 "정렬+가지치기"만 책임, 겹침 판정 자체는 재구현하지 않음).

### 테스트 전략

여러 개의 AABB(및 Sphere 등 혼합)를 배치해 (1) 겹치는 쌍이 정확히 검출되는지, (2) 겹치지 않는 쌍은 제외되는지, (3) x축상 멀리 떨어진 객체가 조기 break로 걸러지는지 확인. 결과 쌍은 정렬 순서에 의존하므로 테스트는 순서 무관하게(집합/포함 여부로) 비교한다.

---

## Checklist

- [x] `src/collision/sweep_and_prune/SweepAndPrune.h`/`.cpp` 구현
- [x] `CMakeLists.txt`에 `.cpp` + 테스트 파일 추가
- [x] `tests/SweepAndPruneTest.cpp` — 겹침/비겹침/여러 객체 혼합 케이스
- [x] 빌드 + 테스트 통과 확인 (개별 코드 리뷰 생략 — 로드맵 10번까지 완료 후 일괄 진행)

## 승인/검증 기준

- [x] 겹치는 모든 쌍이 정확히 검출되고, 겹치지 않는 쌍은 결과에 없음(4개 테스트 케이스 전부 통과).
- [x] 이 모듈 밖(GJK/EPA 등)을 include/참조하지 않음(독립성) — `SweepAndPrune.h`/`.cpp`는 `geometry/Geometry.h`와 `geometry/Intersections.h`만 include, GJK/EPA 관련 헤더 없음 확인.
