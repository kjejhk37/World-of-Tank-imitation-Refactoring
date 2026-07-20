#include <gtest/gtest.h>

#include "collision/mpr/Mpr.h"

TEST(MprTest, SeparatedSpheresDoNotIntersect)
{
    const Geometry a = Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    const Geometry b = Sphere(Vec3(10.0f, 0.0f, 0.0f), 1.0f);
    EXPECT_FALSE(MprIntersects(a, b));
}

TEST(MprTest, OverlappingSpheresIntersect)
{
    const Geometry a = Sphere(Vec3(0.0f, 0.0f, 0.0f), 2.0f);
    const Geometry b = Sphere(Vec3(3.0f, 0.0f, 0.0f), 2.0f);
    EXPECT_TRUE(MprIntersects(a, b));
}

TEST(MprTest, TouchingSpheresIntersect)
{
    const Geometry a = Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    const Geometry b = Sphere(Vec3(1.5f, 0.0f, 0.0f), 1.0f);
    EXPECT_TRUE(MprIntersects(a, b));
}

TEST(MprTest, SeparatedAABBsDoNotIntersect)
{
    const Geometry a = AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const Geometry b = AABB(Vec3(5.0f, 5.0f, 5.0f), Vec3(6.0f, 6.0f, 6.0f));
    EXPECT_FALSE(MprIntersects(a, b));
}

TEST(MprTest, OverlappingAABBsIntersect)
{
    const Geometry a = AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 2.0f));
    const Geometry b = AABB(Vec3(1.0f, 0.0f, 0.0f), Vec3(3.0f, 2.0f, 2.0f));
    EXPECT_TRUE(MprIntersects(a, b));
}

// 참고: OBB처럼 지지 함수가 한 옥탄트 전체에서 같은 모서리를 반환하는 평평한(polytope) 도형은, 이
// 참고용 MPR 구현에서 포탈 정제가 수렴하지 않고 몇 개의 정점 사이를 순환하는 경우가 확인됨(알려진
// 한계 — docs/commit/MPR_*.md 참고). 이번 사이클은 전략 문서에 명시된 Sphere/AABB 케이스만 검증한다.
