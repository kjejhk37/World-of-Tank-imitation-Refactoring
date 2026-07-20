../brainstorming/충돌감지_알고리즘_20260720_2119.md

# Strategy: Voxel 공간 / Spatial Hashing (로드맵 6번)

## 적용 방식

broad-phase 알고리즘. 둘 다 "월드를 균일 격자로 나눠 셀별 오브젝트 목록 관리"라는 같은 아이디어지만, 실제 차이는 셀 저장 방식이다 — Voxel은 월드 경계가 고정이라는 전제하에 배열로 직접 인덱싱하고, Spatial Hashing은 경계 없는(무한) 월드를 가정해 정수 셀 좌표를 해시맵 키로 쓴다. 브레인스토밍에서 "같은 카테고리라 한 사이클에 묶는 안"으로 확정된 대로, 이번 사이클에서 두 개의 독립된 작은 클래스로 구현한다(서로도 참조하지 않음 — 요구사항 1).

### 파일 구성

- `src/collision/voxel_grid/VoxelGrid.h`/`.cpp` — 고정 월드 경계 + `cellsPerAxis`로 배열 기반 격자.
- `src/collision/spatial_hash/SpatialHash.h`/`.cpp` — 셀 크기만으로 무한 월드를 다루는 해시맵 기반 격자.

두 클래스 모두 공통 인터페이스: 생성자(`Geometry` 목록 + 격자 파라미터) + `std::vector<std::pair<size_t,size_t>> FindOverlappingPairs() const` — BVH/R-Tree/SAP와 동일한 반환 형태라 세 부류(정렬 기반/트리 기반/격자 기반) 비교가 가능하다.

### 알고리즘 개요

- 각 객체의 `GetBounds()`로 AABB를 구하고, 그 AABB가 걸치는 모든 셀(범위)에 객체 index를 등록한다(한 객체가 여러 셀에 걸칠 수 있음 — 셀 경계에 걸친 객체를 놓치지 않기 위함).
- 쿼리 시 각 셀 내부의 객체들을 브루트포스로 쌍 비교하되, 실제 겹침은 기존 `Intersects(AABB,AABB)`로 최종 확인한다(셀이 겹친다고 AABB가 실제로 겹치는 건 아님 — 셀은 후보만 좁힌다).
- 한 쌍이 여러 셀에 걸쳐 중복 등록될 수 있으므로, 결과를 정렬 후 `std::unique`로 중복 제거한다.
- `VoxelGrid`는 월드 경계 밖으로 벗어난 좌표를 가장 가까운 경계 셀로 clamp한다(고정 배열이라 범위 밖 인덱스를 만들 수 없음 — 이번 사이클 범위에서 허용하는 단순화).

### 테스트 전략

BVH/SAP/R-Tree와 같은 배치로 동일한 논리적 결과를 검증. 추가로 셀 경계에 걸친 객체가 인접 셀의 객체와도 정상적으로 겹침 검출되는지(멀티 셀 등록 확인) 케이스를 포함한다.

---

## Checklist

- [x] `src/collision/voxel_grid/VoxelGrid.h`/`.cpp` 구현
- [x] `src/collision/spatial_hash/SpatialHash.h`/`.cpp` 구현
- [x] `CMakeLists.txt`에 2개 `.cpp` + 테스트 파일 2개 추가
- [x] `tests/VoxelGridTest.cpp`, `tests/SpatialHashTest.cpp` — 겹침/비겹침/셀 경계 걸침 케이스
- [x] 빌드 + 테스트 통과 확인 (개별 코드 리뷰 생략 — 로드맵 10번까지 완료 후 일괄 진행)

## 승인/검증 기준

- [x] 두 모듈 모두 겹치는 모든 쌍이 정확히 검출되고, 겹치지 않는 쌍/중복 쌍이 없음(각 3~4개 테스트 케이스 전부 통과).
- [x] 셀 경계에 걸친 객체의 겹침도 정상 검출됨(멀티 셀 등록 검증 — 양쪽 모듈 모두 전용 테스트로 확인). 추가로 SpatialHash는 음수 좌표(무한 월드)에서도 정상 동작함을 별도 확인.
- [x] 두 모듈이 서로를 포함한 다른 알고리즘 모듈을 include하지 않음(독립성) — 둘 다 `geometry/Geometry.h`, `geometry/Intersections.h`만 include.
