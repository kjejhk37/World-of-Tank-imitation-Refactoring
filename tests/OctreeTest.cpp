#include <gtest/gtest.h>

#include <algorithm>

#include "collision/octree/Octree.h"

namespace
{
    bool ContainsPair(const std::vector<std::pair<size_t, size_t>>& pairs, size_t a, size_t b)
    {
        const size_t first = std::min(a, b);
        const size_t second = std::max(a, b);
        return std::find(pairs.begin(), pairs.end(), std::pair<size_t, size_t>(first, second)) != pairs.end();
    }

    const AABB kWorldBounds(Vec3(0.0f, 0.0f, 0.0f), Vec3(20.0f, 20.0f, 20.0f));
}

TEST(OctreeTest, FindsOverlappingPairsAndExcludesNonOverlapping)
{
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));         // 0
    objects.push_back(AABB(Vec3(0.5f, 0.5f, 0.5f), Vec3(1.5f, 1.5f, 1.5f)));         // 1 - overlaps 0
    objects.push_back(AABB(Vec3(15.0f, 15.0f, 15.0f), Vec3(16.0f, 16.0f, 16.0f)));   // 2 - isolated
    objects.push_back(Sphere(Vec3(0.2f, 0.2f, 0.2f), 0.1f));                        // 3 - inside 0 only

    const Octree tree(objects, kWorldBounds, 5);
    const auto pairs = tree.FindOverlappingPairs();

    EXPECT_EQ(pairs.size(), 2u);
    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
    EXPECT_TRUE(ContainsPair(pairs, 0, 3));
    EXPECT_FALSE(ContainsPair(pairs, 1, 3));
    EXPECT_FALSE(ContainsPair(pairs, 2, 0));
}

TEST(OctreeTest, ObjectStraddlingCenterStaysAtParentAndStillOverlapsDeeperObject)
{
    // 월드 중심(10,10,10)을 걸치는 객체(A)는 자식으로 내려가지 못하고 루트에 남아야 한다.
    // 자식 쪽에만 있는 객체(C)와 A가 실제로 겹치는지(부모-자손 검사 경로) 확인한다.
    std::vector<Geometry> objects;
    objects.push_back(AABB(Vec3(9.0f, 9.0f, 9.0f), Vec3(11.0f, 11.0f, 11.0f)));   // 0 (A) - 중심선 걸침
    objects.push_back(AABB(Vec3(10.5f, 10.5f, 10.5f), Vec3(10.8f, 10.8f, 10.8f)));  // 1 - A와 겹침, 자식 옥탄트 안
    // 채우기용 — 5개 추가해 분할이 실제로 일어나게 함(kMaxLocalBeforeSubdivide=4)
    for (int i = 0; i < 5; ++i)
    {
        const float offset = static_cast<float>(i) * 0.01f;
        objects.push_back(AABB(Vec3(10.6f + offset, 10.6f, 10.6f), Vec3(10.65f + offset, 10.65f, 10.65f)));
    }

    const Octree tree(objects, kWorldBounds, 5);
    const auto pairs = tree.FindOverlappingPairs();

    EXPECT_TRUE(ContainsPair(pairs, 0, 1));
}

TEST(OctreeTest, EmptyObjectListProducesNoPairs)
{
    const std::vector<Geometry> objects;
    const Octree tree(objects, kWorldBounds, 5);
    EXPECT_TRUE(tree.FindOverlappingPairs().empty());
}
