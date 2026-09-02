../strategy/Voxel_SpatialHashing_20260720_2250.md

# Commit: Voxel 공간 / Spatial Hashing

## 구현 결과

- `src/collision/voxel_grid/VoxelGrid.h`/`.cpp` — 고정 월드 경계(`AABB`) + 축당 셀 개수로 배열(`std::vector<std::vector<size_t>>`) 기반 균일 격자를 구성. 경계 밖 좌표는 가장 가까운 경계 셀로 clamp.
- `src/collision/spatial_hash/SpatialHash.h`/`.cpp` — 셀 크기만으로 정수 셀 좌표를 해시맵(`std::unordered_map`) 키로 써서 무한 월드를 지원.
- 둘 다 객체 AABB가 걸치는 모든 셀에 등록(멀티 셀), 쿼리 시 셀 단위 브루트포스 + 기존 `Intersects(AABB,AABB)`로 최종 확인, 중복 쌍은 정렬+`std::unique`로 제거.
- `CMakeLists.txt`: `app_lib`/`tests`에 각각 2개 `.cpp` 추가.
- `tests/VoxelGridTest.cpp`(3개), `tests/SpatialHashTest.cpp`(4개): 겹침/비겹침, 셀 경계 걸침(멀티 셀 등록 검증), 빈 목록, SpatialHash 전용 음수 좌표 케이스.
- 두 모듈은 서로를 포함해 다른 알고리즘 모듈을 전혀 참조하지 않음(독립성 확인).

## 현재 상태에서 확인된 이슈

- [RESOLVED] `Win32WindowTest.PumpMessagesReturnsTrueWhenNoQuitPosted` 1건 실패 — 이전 사이클들에서도 동일하게 보고된 기존 이슈, 이번 사이클과 무관.
  → resolved: `tests/Win32WindowTest.cpp`에 픽스처 추가, SetUp()에서 잔여 WM_QUIT 드레인. 227/227 테스트 통과 확인 (2026-07-22).

## 체크리스트/검증 상태

전략 문서(`../strategy/Voxel_SpatialHashing_20260720_2250.md`)의 체크리스트 5개 전부 `[x]`, 승인/검증 기준 3개 전부 충족.

## 다음 단계

코드 리뷰는 생략(로드맵 10번까지 완료 후 일괄 진행), 로드맵 7번(Octree / k-d Tree, 참고용 낮은 우선순위)으로 진행.
