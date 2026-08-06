#include <gtest/gtest.h>

#include <algorithm>

#include "platform/collision/bvh/Bvh.h"

namespace
{
    bool ContainsPair(const std::vector<std::pair<size_t, size_t>>& pairs, size_t a, size_t b)
    {
        const size_t first = std::min(a, b);
        const size_t second = std::max(a, b);
        return std::find(pairs.begin(), pairs.end(), std::pair<size_t, size_t>(first, second)) != pairs.end();
    }
}

TEST(BvhTest, FindsOverlappingPairsAndExcludesNonOverlapping)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));         // 0
    objects.push_back(AABB(Vec3(0.5f, 0.5f, 0.5f), Vec3(1.5f, 1.5f, 1.5f)));         // 1 - overlaps 0
    objects.push_back(AABB(Vec3(5.0f, 5.0f, 5.0f), Vec3(6.0f, 6.0f, 6.0f)));         // 2 - isolated
    objects.push_back(Sphere(Vec3(0.2f, 0.2f, 0.2f), 0.1f));                        // 3 - inside 0 only
    objects.push_back(AABB(Vec3(0.9f, 0.9f, 0.9f), Vec3(2.0f, 2.0f, 2.0f)));         // 4 - overlaps 0 and 1

    const Bvh bvh(objects);
    const auto pairs = bvh.FindOverlappingPairs();

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

TEST(BvhTest, NoObjectsProducesNoPairs)
{
    const std::vector<Geometry> objects;
    const Bvh bvh(objects);
    EXPECT_TRUE(bvh.FindOverlappingPairs().empty());
}

TEST(BvhTest, SingleObjectProducesNoPairs)
{
    std::vector<Geometry> objects;
    objects.push_back(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f));
    const Bvh bvh(objects);
    EXPECT_TRUE(bvh.FindOverlappingPairs().empty());
}

TEST(BvhTest, ManyDisjointObjectsProduceNoPairs)
{
    std::vector<Geometry> objects;
    for (int i = 0; i < 20; ++i)
    {
        const float offset = static_cast<float>(i) * 10.0f;
        objects.push_back(AABB(Vec3(offset, 0.0f, 0.0f), Vec3(offset + 1.0f, 1.0f, 1.0f)));
    }
    const Bvh bvh(objects);
    EXPECT_TRUE(bvh.FindOverlappingPairs().empty());
}
