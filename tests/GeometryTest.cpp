#include <gtest/gtest.h>

#include "geometry/Geometry.h"
#include "math/Quaternion.h"

namespace
{
    void ExpectAABBNearlyEqual(const AABB& actual, const AABB& expected, float epsilon = 1e-4f)
    {
        EXPECT_TRUE(NearlyEqual(actual.min, expected.min, epsilon));
        EXPECT_TRUE(NearlyEqual(actual.max, expected.max, epsilon));
    }
}

// ---- Support(Sphere) ----

TEST(GeometryTest, SphereSupportReturnsPointOnSurfaceAlongDirection)
{
    const Geometry geometry = Sphere(Vec3(0.0f, 0.0f, 0.0f), 2.0f);
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(1.0f, 0.0f, 0.0f)), Vec3(2.0f, 0.0f, 0.0f)));
}

TEST(GeometryTest, SphereBoundsIsCubeAroundCenter)
{
    const Geometry geometry = Sphere(Vec3(1.0f, 2.0f, 3.0f), 2.0f);
    ExpectAABBNearlyEqual(GetBounds(geometry), AABB(Vec3(-1.0f, 0.0f, 1.0f), Vec3(3.0f, 4.0f, 5.0f)));
}

// ---- Support(AABB) / GetBounds(AABB) ----

TEST(GeometryTest, AABBSupportPicksCornerMatchingDirectionSign)
{
    const Geometry geometry = AABB(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(-1.0f, 0.0f, 0.0f)), Vec3(-1.0f, 1.0f, 1.0f)));
}

TEST(GeometryTest, AABBBoundsIsItself)
{
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    const Geometry geometry = box;
    ExpectAABBNearlyEqual(GetBounds(geometry), box);
}

// ---- Support(OBB) / GetBounds(OBB) ----

TEST(GeometryTest, OBBSupportPicksCornerAlongWorldAxis)
{
    const Geometry geometry = OBB(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(1.0f, 0.0f, 0.0f)), Vec3(1.0f, 1.0f, 1.0f)));
}

TEST(GeometryTest, OBBBoundsSwapsExtentsAfterNinetyDegreeRotation)
{
    Transform transform;
    transform.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 90.0f);
    const Geometry geometry = OBB(transform, Vec3(1.0f, 2.0f, 3.0f));
    ExpectAABBNearlyEqual(GetBounds(geometry), AABB(Vec3(-3.0f, -2.0f, -1.0f), Vec3(3.0f, 2.0f, 1.0f)));
}

// ---- Support(Capsule) / GetBounds(Capsule) ----

TEST(GeometryTest, CapsuleSupportExtendsFromNearestEndpoint)
{
    const Geometry geometry = Capsule(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 1.0f);
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(0.0f, 0.0f, 1.0f)), Vec3(0.0f, 0.0f, 6.0f)));
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(0.0f, 0.0f, -1.0f)), Vec3(0.0f, 0.0f, -1.0f)));
}

TEST(GeometryTest, CapsuleBoundsEnclosesBothEndpointsPlusRadius)
{
    const Geometry geometry = Capsule(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 1.0f);
    ExpectAABBNearlyEqual(GetBounds(geometry), AABB(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 6.0f)));
}

// ---- Support(Cylinder) / GetBounds(Cylinder) ----

TEST(GeometryTest, CylinderSupportAlongAxisReturnsCapCenter)
{
    const Geometry geometry = Cylinder(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 2.0f);
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(0.0f, 0.0f, 1.0f)), Vec3(0.0f, 0.0f, 5.0f)));
}

TEST(GeometryTest, CylinderSupportPerpendicularToAxisExtendsByRadius)
{
    const Geometry geometry = Cylinder(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 2.0f);
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(1.0f, 0.0f, 0.0f)), Vec3(2.0f, 0.0f, 5.0f)));
}

TEST(GeometryTest, CylinderBoundsEnclosesBothEndpointsPlusRadius)
{
    const Geometry geometry = Cylinder(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 5.0f), 2.0f);
    ExpectAABBNearlyEqual(GetBounds(geometry), AABB(Vec3(-2.0f, -2.0f, -2.0f), Vec3(2.0f, 2.0f, 7.0f)));
}

// ---- Support(Mesh) / GetBounds(Mesh) ----

TEST(GeometryTest, MeshSupportReturnsFarthestVertexAlongDirection)
{
    Mesh mesh;
    mesh.vertices = {Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)};
    const Geometry geometry = mesh;
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(0.0f, 0.0f, 2.0f)), Vec3(0.0f, 0.0f, 1.0f)));
}

TEST(GeometryTest, MeshBoundsEnclosesAllVertices)
{
    Mesh mesh;
    mesh.vertices = {Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)};
    const Geometry geometry = mesh;
    ExpectAABBNearlyEqual(GetBounds(geometry), AABB(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f)));
}

TEST(GeometryTest, EmptyMeshSupportReturnsZeroWithoutCrashing)
{
    const Geometry geometry = Mesh();
    EXPECT_TRUE(NearlyEqual(Support(geometry, Vec3(1.0f, 0.0f, 0.0f)), Vec3::Zero()));
}

TEST(GeometryTest, EmptyMeshBoundsIsDegenerateAtOrigin)
{
    const Geometry geometry = Mesh();
    ExpectAABBNearlyEqual(GetBounds(geometry), AABB(Vec3::Zero(), Vec3::Zero()));
}
