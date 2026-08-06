#include <gtest/gtest.h>

#include <algorithm>

#include "platform/collision/voxel_grid/VoxelGrid.h"

namespace
{
    bool ContainsPair(const std::vector<std::pair<size_t, size_t>>& pairs, size_t a, size_t b)
    {
        const size_t first = std::min(a, b);
        const size_t second = std::max(a, b);
        return std::find(pairs.begin(), pairs.end(), std::pair<size_t, size_t>(first, second)) != pairs.end();
    }

    const AABB kWorldBounds(Vec3(0.0f, 0.0f, 0.0f), Vec3(20.0f, 20.0f, 20.0f));
    constexpr int kCellsPerAxis = 10;  // cellSize = 2
}

TEST(VoxelGridTest, FindsOverlappingPairsAndExcludesNonOverlapping)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));  // 0
    objects.push_back(AABB(Vec3(0.5f, 0.5f, 0.5f), Vec3(1.5f, 1.5f, 1.5f)));  // 1 - overlaps 0
    objects.push_back(AABB(Vec3(15.0f, 15.0f, 15.0f), Vec3(16.0f, 16.0f, 16.0f)));  // 2 - isolated
    objects.push_back(Sphere(Vec3(0.2f, 0.2f, 0.2f), 0.1f));                       // 3 - inside 0 only

    const VoxelGrid grid(objects, kWorldBounds, kCellsPerAxis);
    const auto pairs = grid.FindOverlappingPairs();

    EXPECT_EQ(pairs.size(), 2u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
    EXPECT_TRUE(ContainsPair(pairs, 0, 3));
    EXPECT_FALSE(ContainsPair(pairs, 1, 3));
    EXPECT_FALSE(ContainsPair(pairs, 2, 0));
}

TEST(VoxelGridTest, ObjectStraddlingCellBoundaryStillDetectsOverlapInFarCell)
{
    // A는 x=2 셀 경계를 걸쳐 cell0/cell1 둘 다에 등록되어야 한다. C는 cell1에만 있고 A와만 겹친다 —
    // A가 cell1에도 등록되지 않으면(멀티 셀 등록 버그) 이 겹침을 놓친다.
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(1.5f, 1.0f, 1.0f), Vec3(3.5f, 2.0f, 2.0f)));  // 0 (A) - cell0, cell1 걸침
    objects.push_back(AABB(Vec3(3.0f, 1.0f, 1.0f), Vec3(3.2f, 2.0f, 2.0f)));  // 1 (C) - cell1에만 존재, A와 겹침

    const VoxelGrid grid(objects, kWorldBounds, kCellsPerAxis);
    const auto pairs = grid.FindOverlappingPairs();

    EXPECT_EQ(pairs.size(), 1u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
}

TEST(VoxelGridTest, ObjectsClampedToSameBoundaryCellAreNotFalselyReportedAsOverlapping)
{
    // 둘 다 worldBounds(x<=20) 밖(x=25대, x=30대)에 있어 x 셀 인덱스가 똑같이 마지막 셀(9)로 clamp되고
    // y/z도 겹치지만, 실제 AABB는 x축에서 전혀 겹치지 않는다 — clamp로 같은 셀에 묶여도 최종
    // Intersects() 확인 덕분에 거짓 겹침으로 보고되지 않아야 한다.
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(25.0f, 5.0f, 5.0f), Vec3(26.0f, 6.0f, 6.0f)));  // 0 - x clamp -> cell9
    objects.push_back(AABB(Vec3(30.0f, 5.0f, 5.0f), Vec3(31.0f, 6.0f, 6.0f)));  // 1 - x clamp -> cell9, 실제로는 안 겹침

    const VoxelGrid grid(objects, kWorldBounds, kCellsPerAxis);
    EXPECT_TRUE(grid.FindOverlappingPairs().empty());
}

TEST(VoxelGridTest, EmptyObjectListProducesNoPairs)
{
    const std::vector<Geometry> objects;
    const VoxelGrid grid(objects, kWorldBounds, kCellsPerAxis);
    EXPECT_TRUE(grid.FindOverlappingPairs().empty());
}
