#pragma once

#include <utility>
#include <vector>

#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: Voxel 공간(균일 격자, 배열 기반) broad-phase. 월드 경계가 고정되어 있다는 전제하에
//              셀을 1차원 배열로 직접 인덱싱한다 — 무한 월드를 가정하고 해시맵을 쓰는 SpatialHash와
//              대비되는 지점.
// Input: 생성자 - Geometry 목록, 월드 전체를 감싸는 worldBounds, 각 축당 셀 개수(cellsPerAxis)
// Output: FindOverlappingPairs() - 목록 내 index 쌍(first < second), AABB가 겹치는 쌍만
// Notes: GJK/EPA/SAP/BVH/RTree/SpatialHash 등 다른 알고리즘 모듈을 참조하지 않는다(독립성). worldBounds
//        밖으로 벗어난 좌표는 가장 가까운 경계 셀로 clamp한다(고정 배열이라 범위 밖 인덱스 불가 — 단순화).
// Date: 2026-07-20
class VoxelGrid
{
public:
    VoxelGrid(const std::vector<Geometry>& objects, const AABB& worldBounds, int cellsPerAxis);

    std::vector<std::pair<size_t, size_t>> FindOverlappingPairs() const;

private:
    int CellIndex(int x, int y, int z) const;
    int ClampCell(float relative, float cellSize) const;

    AABB worldBounds_;
    Vec3 cellSize_;
    int cellsPerAxis_;
    std::vector<AABB> objectBounds_;
    std::vector<std::vector<size_t>> cells_;
};
