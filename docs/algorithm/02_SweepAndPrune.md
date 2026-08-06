# Sweep and Prune — 정렬만으로 후보 쌍 추리기

**분류**: broad-phase (다수 객체 중 실제로 검사할 만한 쌍만 추리기).
**선행 지식**: [00_Geometry_공용입력타입.md](00_Geometry_공용입력타입.md)의 `GetBounds()` 함수.

## 왜 필요한가

객체가 N개면 모든 쌍을 검사하는 데 N×(N-1)/2번의 비교가 필요하다(객체 1000개면 약 50만 번).
narrow-phase(GJK/EPA 등)는 정확하지만 느리므로, 그 전에 "이 두 객체는 애초에 가까이 있지도 않다"를 빠르게 걸러내는 단계가 필요하다 — 이게 broad-phase다.
Sweep and Prune(SAP)은 broad-phase 중 가장 단순한 방법이다.

## 핵심 아이디어

3차원에서 두 AABB가 겹치려면 x, y, z **세 축 모두**에서 구간이 겹쳐야 한다.
반대로 말하면, **한 축에서만 안 겹쳐도 그 쌍은 확실히 안 겹친다**.
그래서 "한 축을 기준으로 정렬해두면, 그 축에서 이미 멀어진 쌍은 다른 축을 볼 필요도 없이 걸러낼 수 있다"는 게 SAP의 핵심이다.

## 알고리즘

1. 모든 객체의 `GetBounds()`로 AABB를 구한다.
2. `AABB.min.x` 기준으로 오름차순 정렬한다(이 프로젝트는 항상 x축 하나만 사용 — 분산이 가장 큰 축을 고르는 최적화는 범위 밖).
3. 정렬된 순서로 `i`를 순회하며, `i` 다음의 `j`에 대해 `entries[j].min.x > entries[i].max.x`이면 **그 이후의 모든 j도 절대 겹칠 수 없으므로** 즉시 `break`한다(정렬되어 있기 때문에 안전).
4. break 전까지의 `j`에 대해서만 실제 3축 AABB 겹침(`Intersects(AABB,AABB)`)을 확인해 후보 쌍에 추가한다.

```
정렬 후: [A(0~2)] [B(1~3)] [C(5~6)] [D(10~12)]
i=A: j=B(겹침 가능, 확인) → 겹침. j=C(5 > A.max=2) → break. (D는 볼 필요도 없음)
i=B: j=C(5 > B.max=3) → break.
i=C: j=D(10 > C.max=6) → break.
```

## 복잡도

- 정렬: O(n log n).
- 스캔: 최선의 경우(객체가 공간에 고르게 퍼져 있을 때) 거의 O(n)에 가깝지만, 최악의 경우(모든 객체가 한 축에서 겹침) O(n²)까지 나빠질 수 있다.
- 그래도 실제 판정(`Intersects`)을 하는 횟수 자체를 크게 줄여준다는 게 핵심.

## 이 프로젝트에서의 구현

- 파일: `src/platform/collision/sweep_and_prune/SweepAndPrune.h`/`.cpp`
- 함수 하나: `FindOverlappingPairs(const std::vector<Geometry>&)` — 클래스가 아니라 **자유 함수**다.
- BVH/R-Tree와 비교했을 때 이 부분이 SAP의 정체성이다: SAP는 "매번 새로 정렬하는 1회성 계산"이라 트리처럼 미리 지어두고 재사용할 자료구조가 없다.
- 실제 3축 겹침 판정은 새로 만들지 않고 기존 `geometry/Intersections.h`의 `Intersects(AABB,AABB)`를 재사용한다.

## 언제 쓰는가

- 구현이 가장 단순한 broad-phase — BVH/R-Tree보다 코드량이 훨씬 적다.
- 객체들이 한 축(예: 도로를 따라 늘어선 차량들)을 따라 자연스럽게 퍼져 있는 상황에 특히 유리.
- 실무에서도(Bullet의 `btAxisSweep3` 등) 표준으로 쓰이는 기법.
- 단점: 한 축에 객체가 몰리는 배치(예: 넓은 평면 위에 흩어진 경우)에서는 비효율적 — 이럴 때는 BVH/R-Tree/격자 계열이 유리.
