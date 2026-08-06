#include <gtest/gtest.h>

#include <algorithm>

#include "platform/collision/fcl/Fcl.h"

namespace
{
    bool ContainsPair(const std::vector<CollisionPair>& pairs, size_t a, size_t b)
    {
        const size_t first = std::min(a, b);
        const size_t second = std::max(a, b);
        return std::find_if(pairs.begin(), pairs.end(), [&](const CollisionPair& p) {
                   return std::min(p.indexA, p.indexB) == first && std::max(p.indexA, p.indexB) == second;
               }) != pairs.end();
    }
}

TEST(FclTest, BroadPhaseFalsePositiveIsFilteredByNarrowPhase)
{
    std::vector<Geometry> objects;
    objects.push_back(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f));       // 0
    objects.push_back(Sphere(Vec3(1.9f, 1.9f, 0.0f), 1.0f));       // 1 - AABB만 0과 겹침, 실제 구는 안 닿음
    objects.push_back(Sphere(Vec3(0.5f, 0.0f, 0.0f), 1.0f));       // 2 - 0과 실제로 겹침
    objects.push_back(Sphere(Vec3(100.0f, 100.0f, 100.0f), 1.0f));  // 3 - 완전히 고립

    const Fcl fcl(objects);
    const auto results = fcl.DetectCollisions();

    EXPECT_EQ(results.size(), 1u);
    EXPECT_TRUE(ContainsPair(results, 0, 2));
    EXPECT_FALSE(ContainsPair(results, 0, 1));
    EXPECT_FALSE(ContainsPair(results, 1, 2));
}

TEST(FclTest, CollidingPairPenetrationDepthMatchesAnalyticValue)
{
    std::vector<Geometry> objects;
    objects.push_back(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f));
    objects.push_back(Sphere(Vec3(0.5f, 0.0f, 0.0f), 1.0f));

    const Fcl fcl(objects);
    const auto results = fcl.DetectCollisions();

    ASSERT_EQ(results.size(), 1u);
    EXPECT_NEAR(results[0].penetration.depth, 1.5f, 1e-2f);
}

TEST(FclTest, EmptyObjectListProducesNoCollisions)
{
    const std::vector<Geometry> objects;
    const Fcl fcl(objects);
    EXPECT_TRUE(fcl.DetectCollisions().empty());
}
