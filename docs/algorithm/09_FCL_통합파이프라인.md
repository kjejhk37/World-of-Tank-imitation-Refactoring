# 커스텀 FCL 통합 파이프라인 — Broad-phase와 Narrow-phase 엮기

**분류**: 통합 파이프라인. 이번 로드맵의 마지막 항목이자 유일하게 다른 모듈에 의존하는 모듈.
**선행 지식**: [01_GJK_EPA.md](01_GJK_EPA.md), [03_BVH.md](03_BVH.md) — 이 둘을 그대로 엮어서 만든다.

## FCL이란

FCL(Flexible Collision Library)은 로보틱스 분야에서 널리 쓰이는 실제 오픈소스 충돌 감지 라이브러리 이름이다.
이 라이브러리가 실제로 하는 일은 **"broad-phase로 후보를 값싸게 추리고, narrow-phase로 정확하게 확정하는" 파이프라인**을 하나로 묶어 제공하는 것이다 — 이 프로젝트에서는 실제 FCL 라이브러리를 가져다 쓰지 않고, 지금까지 만든 모듈을 그대로 조립해서 같은 개념을 직접 구현했다.

## 왜 이 모듈만 다른 모듈에 의존하는가

이 프로젝트의 다른 9개 알고리즘(GJK/EPA, SAP, BVH, R-Tree, Voxel, Spatial Hashing, Octree, k-d Tree, MPR, CCD)은 전부 서로 독립이다 — 어느 하나가 고장 나도 다른 것들은 영향받지 않고, 각각 `Geometry`(공용 입력 타입)에만 의존한다.
하지만 "broad-phase + narrow-phase를 엮은 파이프라인"이라는 요청 자체가 **다른 두 모듈을 엮으라는 요청**이므로, 이 모듈만은 예외로 `Bvh`와 `GjkEpa`를 직접 참조한다(브레인스토밍 단계에서 합의된 명시적 예외).
그래서 로드맵상 항상 마지막에 왔다 — 의존하는 모듈들이 먼저 완성돼 있어야 하기 때문이다.

## 동작 방식

```cpp
class Fcl {
    explicit Fcl(const std::vector<Geometry>& objects);
    std::vector<CollisionPair> DetectCollisions() const;
};
```

1. **Broad-phase**: 생성자에 전달된 `Geometry` 목록으로 `Bvh`를 빌드하고, `FindOverlappingPairs()`로 AABB가 겹치는 후보 쌍을 추린다.
2. **Narrow-phase**: 각 후보 쌍에 `ComputePenetration()`(GJK+EPA)을 호출한다 — AABB는 겹쳤지만 실제 도형은 안 닿는 경우(broad-phase의 전형적인 오탐)를 걸러내고, 실제로 충돌하는 쌍만 침투 법선+깊이와 함께 최종 결과(`CollisionPair`)에 남긴다.

## 왜 이 2단계 구조가 필요한가 (오탐 예시)

두 구가 AABB(정육면체 상자)로는 대각선 방향에서 겹칠 수 있다 — 예를 들어 반지름 1짜리 두 구가 중심 거리 2.687만큼 떨어져 있으면 실제로는 안 닿지만(반지름 합 2보다 크므로), 그 구들을 감싸는 정육면체 AABB끼리는 모서리가 겹칠 수 있다.
BVH(broad-phase)만 쓰면 이런 쌍도 "후보"로 남는다.
GJK+EPA(narrow-phase)가 이걸 최종적으로 "실제로는 안 닿았다"고 걸러내는 게 이 파이프라인의 핵심 가치다 — broad-phase 혼자서는 못 하는 일이고, narrow-phase만 모든 쌍에 다 돌리면 너무 느리다.

## 이 프로젝트에서의 구현

- 파일: `src/platform/collision/fcl/Fcl.h`/`.cpp`
- `struct CollisionPair { size_t indexA; size_t indexB; PenetrationInfo penetration; };`
- 테스트에서 "AABB는 겹치지만 실제 구는 안 닿는 쌍이 최종 결과에서 제외되는지"를 직접 검증했다(`tests/FclTest.cpp`).

## 전체 그림

```
Geometry(공용 입력)
   │
   ├─ 9개 독립 알고리즘 (GJK/EPA, SAP, BVH, R-Tree, Voxel, SpatialHash, Octree, k-d Tree, MPR, CCD)
   │     — 전부 Geometry에만 의존, 서로 모름
   │
   └─ Fcl (유일한 예외)
         ├─ Bvh 로 broad-phase
         └─ GjkEpa(ComputePenetration) 로 narrow-phase 확정
```

이 구조가 이번 로드맵 전체가 보여주려던 것 — "각 알고리즘을 독립적으로 이해하고 구현한 뒤, 필요할 때만 명시적으로 조합한다"는 설계 원칙이다.
