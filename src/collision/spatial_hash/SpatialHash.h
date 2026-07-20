#pragma once

#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>

#include "geometry/Geometry.h"

// Author: Claude
// Description: Spatial Hashing broad-phase. VoxelGrid와 같은 균일 격자 아이디어지만, 고정 배열 대신
//              정수 셀 좌표를 해시맵 키로 써서 월드 경계 없이(무한 월드) 동작한다.
// Input: 생성자 - Geometry 목록, 셀 한 변의 길이(cellSize)
// Output: FindOverlappingPairs() - 목록 내 index 쌍(first < second), AABB가 겹치는 쌍만
// Notes: GJK/EPA/SAP/BVH/RTree/VoxelGrid 등 다른 알고리즘 모듈을 참조하지 않는다(독립성).
// Date: 2026-07-20
class SpatialHash
{
public:
    SpatialHash(const std::vector<Geometry>& objects, float cellSize);

    std::vector<std::pair<size_t, size_t>> FindOverlappingPairs() const;

private:
    struct CellKey
    {
        int x, y, z;
        bool operator==(const CellKey& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct CellKeyHash
    {
        size_t operator()(const CellKey& key) const
        {
            size_t seed = std::hash<int>()(key.x);
            seed ^= std::hash<int>()(key.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<int>()(key.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    int CellCoord(float value) const;

    float cellSize_;
    std::vector<AABB> objectBounds_;
    std::unordered_map<CellKey, std::vector<size_t>, CellKeyHash> cells_;
};
