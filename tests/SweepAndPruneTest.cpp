#include <gtest/gtest.h>

#include <algorithm>

#include "collision/sweep_and_prune/SweepAndPrune.h"

namespace
{
    bool ContainsPair(const std::vector<std::pair<size_t, size_t>>& pairs, size_t a, size_t b)
    {
        const size_t first = std::min(a, b);
        const size_t second = std::max(a, b);
        return std::find(pairs.begin(), pairs.end(), std::pair<size_t, size_t>(first, second)) != pairs.end();
    }
}

TEST(SweepAndPruneTest, FindsOverlappingPairsAndExcludesNonOverlapping)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));         // 0
    objects.push_back(AABB(Vec3(0.5f, 0.5f, 0.5f), Vec3(1.5f, 1.5f, 1.5f)));         // 1 - overlaps 0
    objects.push_back(AABB(Vec3(5.0f, 5.0f, 5.0f), Vec3(6.0f, 6.0f, 6.0f)));         // 2 - isolated
    objects.push_back(Sphere(Vec3(0.2f, 0.2f, 0.2f), 0.1f));                        // 3 - inside 0 only
    objects.push_back(AABB(Vec3(0.9f, 0.9f, 0.9f), Vec3(2.0f, 2.0f, 2.0f)));         // 4 - overlaps 0 and 1

    const auto pairs = FindOverlappingPairs(objects);

    EXPECT_EQ(pairs.size(), 4u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
    EXPECT_TRUE(ContainsPair(pairs, 0, 3));
    EXPECT_TRUE(ContainsPair(pairs, 0, 4));
    EXPECT_TRUE(ContainsPair(pairs, 1, 4));
    EXPECT_FALSE(ContainsPair(pairs, 1, 3));
    EXPECT_FALSE(ContainsPair(pairs, 3, 4));
    EXPECT_FALSE(ContainsPair(pairs, 2, 0));
    EXPECT_FALSE(ContainsPair(pairs, 2, 1));
}

TEST(SweepAndPruneTest, NoObjectsProducesNoPairs)
{
    const std::vector<Geometry> objects;
    EXPECT_TRUE(FindOverlappingPairs(objects).empty());
}

TEST(SweepAndPruneTest, SingleObjectProducesNoPairs)
{
    std::vector<Geometry> objects;
    objects.push_back(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f));
    EXPECT_TRUE(FindOverlappingPairs(objects).empty());
}

TEST(SweepAndPruneTest, FarApartAlongSweepAxisAreExcludedByEarlyBreak)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));
    objects.push_back(AABB(Vec3(100.0f, 0.0f, 0.0f), Vec3(101.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(FindOverlappingPairs(objects).empty());
}
