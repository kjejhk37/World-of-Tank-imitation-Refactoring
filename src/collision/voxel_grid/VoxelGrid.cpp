#include "collision/voxel_grid/VoxelGrid.h"

#include <algorithm>
#include <cmath>

#include "geometry/Intersections.h"

// Author: Claude
// Description: VoxelGrid 멤버 함수 구현 — 배열 기반 균일 격자에 객체를 등록하고, 셀 단위로 후보 쌍을
//              추린 뒤 실제 겹침은 재사용한 Intersects(AABB,AABB)로 확인한다.
// Input/Output: (VoxelGrid.h 참고)
// Date: 2026-07-20

VoxelGrid::VoxelGrid(const std::vector<Geometry>& objects, const AABB& worldBounds, int cellsPerAxis)
    : worldBounds_(worldBounds), cellsPerAxis_(cellsPerAxis)
{
    const Vec3 worldSize = worldBounds_.max - worldBounds_.min;
    cellSize_ = Vec3(worldSize.x / static_cast<float>(cellsPerAxis_), worldSize.y / static_cast<float>(cellsPerAxis_),
                      worldSize.z / static_cast<float>(cellsPerAxis_));

    cells_.resize(static_cast<size_t>(cellsPerAxis_) * cellsPerAxis_ * cellsPerAxis_);

    objectBounds_.reserve(objects.size());
    for (const Geometry& object : objects)
    {
        objectBounds_.push_back(GetBounds(object));
    }

    for (size_t index = 0; index < objectBounds_.size(); ++index)
    {
        const AABB& bounds = objectBounds_[index];
        const Vec3 relMin = bounds.min - worldBounds_.min;
        const Vec3 relMax = bounds.max - worldBounds_.min;

        const int xMin = ClampCell(relMin.x, cellSize_.x);
        const int xMax = ClampCell(relMax.x, cellSize_.x);
        const int yMin = ClampCell(relMin.y, cellSize_.y);
        const int yMax = ClampCell(relMax.y, cellSize_.y);
        const int zMin = ClampCell(relMin.z, cellSize_.z);
        const int zMax = ClampCell(relMax.z, cellSize_.z);

        for (int x = xMin; x <= xMax; ++x)
        {
            for (int y = yMin; y <= yMax; ++y)
            {
                for (int z = zMin; z <= zMax; ++z)
                {
                    cells_[static_cast<size_t>(CellIndex(x, y, z))].push_back(index);
                }
            }
        }
    }
}

int VoxelGrid::ClampCell(float relative, float cellSize) const
{
    const int cell = static_cast<int>(std::floor(relative / cellSize));
    return std::clamp(cell, 0, cellsPerAxis_ - 1);
}

int VoxelGrid::CellIndex(int x, int y, int z) const
{
    return (x * cellsPerAxis_ + y) * cellsPerAxis_ + z;
}

std::vector<std::pair<size_t, size_t>> VoxelGrid::FindOverlappingPairs() const
{
    std::vector<std::pair<size_t, size_t>> results;
    for (const std::vector<size_t>& cell : cells_)
    {
        for (size_t i = 0; i < cell.size(); ++i)
        {
            for (size_t j = i + 1; j < cell.size(); ++j)
            {
                if (Intersects(objectBounds_[cell[i]], objectBounds_[cell[j]]))
                {
                    const size_t first = std::min(cell[i], cell[j]);
                    const size_t second = std::max(cell[i], cell[j]);
                    results.push_back({first, second});
                }
            }
        }
    }
    std::sort(results.begin(), results.end());
    results.erase(std::unique(results.begin(), results.end()), results.end());
    return results;
}
