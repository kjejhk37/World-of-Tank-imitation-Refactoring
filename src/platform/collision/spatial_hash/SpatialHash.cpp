#include "platform/collision/spatial_hash/SpatialHash.h"

#include <algorithm>
#include <cmath>

#include "platform/geometry/Intersections.h"

// Author: Claude
// Description: SpatialHash 멤버 함수 구현 — 해시맵 기반 균일 격자에 객체를 등록하고, 셀 단위로 후보
//              쌍을 추린 뒤 실제 겹침은 재사용한 Intersects(AABB,AABB)로 확인한다.
// Input/Output: (SpatialHash.h 참고)
// Date: 2026-07-20

SpatialHash::SpatialHash(const std::vector<Geometry>& objects, float cellSize) : cellSize_(cellSize)
{
    objectBounds_.reserve(objects.size());
    for (const Geometry& object : objects)
    {
        objectBounds_.push_back(GetBounds(object));
    }

    for (size_t index = 0; index < objectBounds_.size(); ++index)
    {
        const AABB& bounds = objectBounds_[index];
        const int xMin = CellCoord(bounds.min.x);
        const int xMax = CellCoord(bounds.max.x);
        const int yMin = CellCoord(bounds.min.y);
        const int yMax = CellCoord(bounds.max.y);
        const int zMin = CellCoord(bounds.min.z);
        const int zMax = CellCoord(bounds.max.z);

        for (int x = xMin; x <= xMax; ++x)
        {
            for (int y = yMin; y <= yMax; ++y)
            {
                for (int z = zMin; z <= zMax; ++z)
                {
                    cells_[CellKey{x, y, z}].push_back(index);
                }
            }
        }
    }
}

int SpatialHash::CellCoord(float value) const
{
    return static_cast<int>(std::floor(value / cellSize_));
}

std::vector<std::pair<size_t, size_t>> SpatialHash::FindOverlappingPairs() const
{
    std::vector<std::pair<size_t, size_t>> results;
    for (const auto& [key, indices] : cells_)
    {
        for (size_t i = 0; i < indices.size(); ++i)
        {
            for (size_t j = i + 1; j < indices.size(); ++j)
            {
                if (Intersects(objectBounds_[indices[i]], objectBounds_[indices[j]]))
                {
                    const size_t first = std::min(indices[i], indices[j]);
                    const size_t second = std::max(indices[i], indices[j]);
                    results.push_back({first, second});
                }
            }
        }
    }
    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());
    return results;
}
