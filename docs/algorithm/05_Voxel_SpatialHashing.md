# Voxel 공간 / Spatial Hashing — 균일 격자로 나누기

**분류**: broad-phase.
**선행 지식**: [00_Geometry_공용입력타입.md](00_Geometry_공용입력타입.md)의 `GetBounds()` 함수.

이 둘은 "월드를 균일한 격자로 나눠 셀별로 오브젝트 목록을 관리한다"는 **같은 아이디어**다.
차이는 딱 하나, **셀을 어떻게 저장하느냐**다 — 그래서 이번 사이클에서도 한 쌍으로 묶어 구현했다(단, 코드는 서로 독립).

## 공통 알고리즘

1. 셀 크기(`cellSize`)를 정한다.
2. 각 객체의 `GetBounds()`로 AABB를 구하고, 그 AABB가 걸치는 **모든 셀**에 그 객체의 인덱스를 등록한다(한 객체가 여러 셀에 걸칠 수 있음 — 셀 경계에 걸친 객체를 놓치지 않기 위해 반드시 필요).
3. 쿼리 시 각 셀 내부의 객체들끼리만 브루트포스로 비교하고, 실제 겹침은 기존 `Intersects(AABB,AABB)`로 최종 확인한다.
4. 한 쌍이 여러 셀에 걸쳐 중복 등록될 수 있으므로, 결과를 정렬 후 `std::unique`로 중복 제거한다.

## Voxel Grid — 배열 기반, 고정 경계

- 월드 전체를 감싸는 경계(`worldBounds`)와 축당 셀 개수(`cellsPerAxis`)를 미리 정해야 한다.
- 셀은 `std::vector<std::vector<size_t>>`로, `(x*cellsPerAxis + y)*cellsPerAxis + z` 같은 **직접 배열 인덱싱**으로 접근한다 — 해시 계산 없이 바로 주소를 계산하니 조회가 아주 빠르다.
- `worldBounds` 밖으로 벗어난 좌표는 가장 가까운 경계 셀로 clamp한다(고정 배열이라 범위 밖 인덱스를 만들 수 없기 때문).
- 파일: `src/platform/collision/voxel_grid/VoxelGrid.h`/`.cpp`.

## Spatial Hashing — 해시맵 기반, 무한 월드

- 월드 경계가 필요 없다 — 좌표를 셀 크기로 나눈 정수 좌표(`floor(x/cellSize)` 등)를 키로 삼아 `std::unordered_map`에 저장한다.
- 음수 좌표도 그대로 처리 가능(배열이 아니니 "범위 밖"이라는 개념 자체가 없음) — 이게 Voxel Grid 대비 가장 큰 실전 이점이다.
- 파일: `src/platform/collision/spatial_hash/SpatialHash.h`/`.cpp`.

## 비교

| | Voxel Grid | Spatial Hashing |
|---|---|---|
| 저장 | 고정 크기 배열 | 해시맵(`unordered_map`) |
| 월드 경계 | 미리 정해야 함 | 필요 없음(무한) |
| 조회 속도 | 배열 인덱싱(가장 빠름) | 해시 계산 + 충돌 처리 필요 |
| 메모리 | 셀이 비어 있어도 배열 칸은 차지 | 실제로 쓰는 셀만 메모리 사용 |

## 언제 쓰는가

- 객체가 공간에 비교적 고르게 퍼져 있고, 밀도가 어느 정도 예측 가능할 때(셀 크기를 잘 고르면 조회가 거의 O(1)).
- Voxel Grid는 월드 크기가 고정된 경우(예: 정해진 맵 크기의 전장), Spatial Hashing은 월드가 사실상 무한하거나 크기를 미리 알기 어려운 경우.
- 단점: 객체 밀도가 아주 불균일하면(한 셀에 몰림) 비효율 — 이럴 때는 적응형 구조(Octree)가 유리.
