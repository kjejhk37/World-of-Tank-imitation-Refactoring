#include <gtest/gtest.h>

#include "collision/gjk_epa/GjkEpa.h"

// ---- Sphere-Sphere (해석적 깊이 검증) ----

TEST(GjkEpaTest, SeparatedSpheresDoNotIntersect)
{
    const Geometry a = Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    const Geometry b = Sphere(Vec3(10.0f, 0.0f, 0.0f), 1.0f);
    EXPECT_FALSE(Intersects(a, b));
    EXPECT_FALSE(ComputePenetration(a, b).has_value());
}

TEST(GjkEpaTest, OverlappingSpheresPenetrationMatchesAnalyticDepth)
{
    const Geometry a = Sphere(Vec3(0.0f, 0.0f, 0.0f), 2.0f);
    const Geometry b = Sphere(Vec3(3.0f, 0.0f, 0.0f), 2.0f);
    EXPECT_TRUE(Intersects(a, b));

    const auto penetration = ComputePenetration(a, b);
    ASSERT_TRUE(penetration.has_value());
    EXPECT_NEAR(penetration->depth, 1.0f, 1e-2f);
    EXPECT_NEAR(std::abs(penetration->normal.x), 1.0f, 1e-2f);
    EXPECT_NEAR(penetration->normal.y, 0.0f, 1e-2f);
    EXPECT_NEAR(penetration->normal.z, 0.0f, 1e-2f);
}

// ---- AABB-AABB (해석적 깊이 검증) ----

TEST(GjkEpaTest, SeparatedAABBsDoNotIntersect)
{
    const Geometry a = AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const Geometry b = AABB(Vec3(5.0f, 5.0f, 5.0f), Vec3(6.0f, 6.0f, 6.0f));
    EXPECT_FALSE(Intersects(a, b));
}

TEST(GjkEpaTest, OverlappingAABBsPenetrationMatchesAnalyticDepth)
{
    const Geometry a = AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 2.0f));
    const Geometry b = AABB(Vec3(1.0f, 0.0f, 0.0f), Vec3(3.0f, 2.0f, 2.0f));
    EXPECT_TRUE(Intersects(a, b));

    const auto penetration = ComputePenetration(a, b);
    ASSERT_TRUE(penetration.has_value());
    EXPECT_NEAR(penetration->depth, 1.0f, 1e-2f);
}

// ---- Sphere-AABB (해석적 깊이 검증) ----

TEST(GjkEpaTest, OverlappingSphereAABBPenetrationMatchesAnalyticDepth)
{
    const Geometry sphere = Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.5f);
    const Geometry box = AABB(Vec3(1.0f, -1.0f, -1.0f), Vec3(3.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Intersects(sphere, box));

    const auto penetration = ComputePenetration(sphere, box);
    ASSERT_TRUE(penetration.has_value());
    EXPECT_NEAR(penetration->depth, 0.5f, 1e-2f);
}

// ---- OBB-OBB ----

TEST(GjkEpaTest, OverlappingOBBsIntersectWithExpectedDepth)
{
    const Geometry a = OBB(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    Transform transformB;
    transformB.position = Vec3(1.5f, 0.0f, 0.0f);
    const Geometry b = OBB(transformB, Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Intersects(a, b));

    const auto penetration = ComputePenetration(a, b);
    ASSERT_TRUE(penetration.has_value());
    EXPECT_NEAR(penetration->depth, 0.5f, 1e-2f);
}

TEST(GjkEpaTest, SeparatedOBBsDoNotIntersect)
{
    const Geometry a = OBB(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    Transform transformB;
    transformB.position = Vec3(10.0f, 0.0f, 0.0f);
    const Geometry b = OBB(transformB, Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Intersects(a, b));
}

// ---- Capsule-Sphere ----

TEST(GjkEpaTest, OverlappingCapsuleSphereIntersectWithExpectedDepth)
{
    const Geometry capsule = Capsule(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 1.0f);
    const Geometry sphere = Sphere(Vec3(0.0f, 0.0f, -0.5f), 1.0f);
    EXPECT_TRUE(Intersects(capsule, sphere));

    const auto penetration = ComputePenetration(capsule, sphere);
    ASSERT_TRUE(penetration.has_value());
    EXPECT_NEAR(penetration->depth, 1.5f, 1e-2f);
}

TEST(GjkEpaTest, SeparatedCapsuleSphereDoNotIntersect)
{
    const Geometry capsule = Capsule(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 1.0f);
    const Geometry sphere = Sphere(Vec3(0.0f, 0.0f, -10.0f), 1.0f);
    EXPECT_FALSE(Intersects(capsule, sphere));
}

// ---- Cylinder-Sphere (경계값만 검증, 정확한 깊이는 검증하지 않음) ----

TEST(GjkEpaTest, OverlappingCylinderSphereIntersect)
{
    const Geometry cylinder = Cylinder(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 1.0f);
    const Geometry sphere = Sphere(Vec3(1.2f, 0.0f, 2.0f), 1.0f);
    EXPECT_TRUE(Intersects(cylinder, sphere));
    EXPECT_TRUE(ComputePenetration(cylinder, sphere).has_value());
}

TEST(GjkEpaTest, SeparatedCylinderSphereDoNotIntersect)
{
    const Geometry cylinder = Cylinder(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 1.0f);
    const Geometry sphere = Sphere(Vec3(10.0f, 0.0f, 2.0f), 1.0f);
    EXPECT_FALSE(Intersects(cylinder, sphere));
}

// ---- Mesh-Sphere ----

TEST(GjkEpaTest, OverlappingMeshSphereIntersect)
{
    Mesh triangle;
    triangle.vertices = {Vec3(-5.0f, -5.0f, 0.0f), Vec3(5.0f, -5.0f, 0.0f), Vec3(0.0f, 5.0f, 0.0f)};
    const Geometry mesh = triangle;
    const Geometry sphere = Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    EXPECT_TRUE(Intersects(mesh, sphere));
}

TEST(GjkEpaTest, SeparatedMeshSphereDoNotIntersect)
{
    Mesh triangle;
    triangle.vertices = {Vec3(-5.0f, -5.0f, 0.0f), Vec3(5.0f, -5.0f, 0.0f), Vec3(0.0f, 5.0f, 0.0f)};
    const Geometry mesh = triangle;
    const Geometry sphere = Sphere(Vec3(0.0f, 0.0f, 10.0f), 1.0f);
    EXPECT_FALSE(Intersects(mesh, sphere));
}
