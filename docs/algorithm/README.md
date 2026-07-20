# 충돌 감지 알고리즘 학습 노트

이 폴더는 `docs/brainstorming/충돌감지_알고리즘_20260720_2119.md` 사이클에서 구현한 충돌 감지 알고리즘 10종에 대한 학습용 설명 문서 모음이다.
각 문서는 "무엇을 푸는 문제인가 → 어떻게 푸는가 → 이 프로젝트에서 어떻게 구현했는가 → 언제 쓰는가" 순서로 정리했고, 구현 중 실제로 만난 버그/트레이드오프도 정직하게 기록했다.

## 읽는 순서

1. [00_Geometry_공용입력타입.md](00_Geometry_공용입력타입.md) — **먼저 읽을 것**. 나머지 모든 문서가 `Support()`/`GetBounds()`를 전제로 한다.
2. [01_GJK_EPA.md](01_GJK_EPA.md) — narrow-phase(정확한 충돌+깊이 계산)의 표준.
3. [02_SweepAndPrune.md](02_SweepAndPrune.md) — broad-phase 중 가장 단순한 방법.
4. [03_BVH.md](03_BVH.md) — 트리로 만든 broad-phase, 한 번 짓고 재사용.
5. [04_RTree.md](04_RTree.md) — BVH와 같은 문제를 점진적 삽입으로 푸는 대안.
6. [05_Voxel_SpatialHashing.md](05_Voxel_SpatialHashing.md) — 균일 격자 기반(배열 vs 해시맵).
7. [06_Octree_KdTree.md](06_Octree_KdTree.md) — Voxel/BVH의 변형(참고용).
8. [07_MPR.md](07_MPR.md) — GJK의 대안(참고용, 알려진 한계 있음).
9. [08_ConservativeAdvancement.md](08_ConservativeAdvancement.md) — 연속 충돌 감지(CCD), 터널링 방지.
10. [09_FCL_통합파이프라인.md](09_FCL_통합파이프라인.md) — broad-phase + narrow-phase를 엮은 마지막 조립.

## 분류 한눈에 보기

| 알고리즘 | 분류 | 핵심 자료구조 | 우선순위 |
|---|---|---|---|
| GJK + EPA | narrow-phase | 심플렉스 / 다면체 | 높음 |
| Sweep and Prune | broad-phase | 정렬된 배열 | 높음 |
| BVH | broad-phase | 이진 트리(top-down) | 높음 |
| R-Tree | broad-phase | N-ary 트리(점진적 삽입) | 높음 |
| Voxel Grid | broad-phase | 배열(균일 격자) | 높음 |
| Spatial Hashing | broad-phase | 해시맵(균일 격자) | 높음 |
| Octree | broad-phase | 8분할 적응형 트리 | 참고용 |
| k-d Tree | broad-phase | 이진 트리(축 순환) | 참고용 |
| MPR | narrow-phase | 포탈(삼각형) | 참고용 |
| Conservative Advancement | CCD | 경계 구 근사 | 높음 |
| 커스텀 FCL | 통합 파이프라인 | BVH + GJK/EPA | 마지막(의존) |

## 관련 워크플로 문서

- 브레인스토밍(왜 이 알고리즘들을 골랐는지, GJK/EPA vs IGeometry 설계 논의 등): `docs/brainstorming/충돌감지_알고리즘_20260720_2119.md`
- 각 알고리즘의 전략/구현 체크리스트: `docs/strategy/`
- 각 알고리즘의 구현 결과와 발견된 이슈: `docs/commit/`
- 통합 코드 리뷰(품질 점검 + 아키텍처 다이어그램): `docs/review/충돌감지_알고리즘_통합_20260720_2337.md`
