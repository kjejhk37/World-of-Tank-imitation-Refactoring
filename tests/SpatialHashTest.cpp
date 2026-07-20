#include <gtest/gtest.h>

#include <algorithm>

#include "collision/spatial_hash/SpatialHash.h"

namespace
{
    bool ContainsPair(const std::vector<std::pair<size_t, size_t>>& pairs, size_t a, size_t b)
    {
        const size_t first = std::min(a, b);
        const size_t second = std::max(a, b);
        return std::find(pairs.begin(), pairs.end(), std::pair<size_t, size_t>(first, second)) != pairs.end();
    }

    constexpr float kCellSize = 2.0f;
}

TEST(SpatialHashTest, FindsOverlappingPairsAndExcludesNonOverlapping)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));        // 0
    objects.push_back(AABB(Vec3(0.5f, 0.5f, 0.5f), Vec3(1.5f, 1.5f, 1.5f)));        // 1 - overlaps 0
    objects.push_back(AABB(Vec3(50.0f, 50.0f, 50.0f), Vec3(51.0f, 51.0f, 51.0f)));  // 2 - isolated
    objects.push_back(Sphere(Vec3(0.2f, 0.2f, 0.2f), 0.1f));                        // 3 - inside 0 only

    const SpatialHash hash(objects, kCellSize);
    const auto pairs = hash.FindOverlappingPairs();

    EXPECT_EQ(pairs.size(), 2u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
    EXPECT_TRUE(ContainsPair(pairs, 0, 3));
    EXPECT_FALSE(ContainsPair(pairs, 1, 3));
    EXPECT_FALSE(ContainsPair(pairs, 2, 0));
}

TEST(SpatialHashTest, ObjectStraddlingCellBoundaryStillDetectsOverlapInFarCell)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(1.5f, 1.0f, 1.0f), Vec3(3.5f, 2.0f, 2.0f)));  // 0 (A) - cell0, cell1 걸침
    objects.push_back(AABB(Vec3(3.0f, 1.0f, 1.0f), Vec3(3.2f, 2.0f, 2.0f)));  // 1 (C) - cell1에만 존재

    const SpatialHash hash(objects, kCellSize);
    const auto pairs = hash.FindOverlappingPairs();

    EXPECT_EQ(pairs.size(), 1u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
}

TEST(SpatialHashTest, NegativeCoordinatesWorkWithoutWorldBounds)
{
    // VoxelGrid와 달리 SpatialHash는 고정 월드 경계가 없어 음수 좌표도 그대로 처리할 수 있어야 한다.
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(-100.0f, -100.0f, -100.0f), Vec3(-99.0f, -99.0f, -99.0f)));  // 0
    objects.push_back(AABB(Vec3(-99.5f, -99.5f, -99.5f), Vec3(-98.5f, -98.5f, -98.5f)));      // 1 - overlaps 0

    const SpatialHash hash(objects, kCellSize);
    const auto pairs = hash.FindOverlappingPairs();

    EXPECT_EQ(pairs.size(), 1u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
}

TEST(SpatialHashTest, EmptyObjectListProducesNoPairs)
{
    const std::vector<Geometry> objects;
    const SpatialHash hash(objects, kCellSize);
    EXPECT_TRUE(hash.FindOverlappingPairs().empty());
}
