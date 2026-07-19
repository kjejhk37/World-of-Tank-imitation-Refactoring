#include <gtest/gtest.h>

#include "math/Transform.h"

TEST(TransformTest, DefaultTransformToMatrixIsIdentity)
{
    const Transform transform;
    const Matrix4x4 matrix = transform.ToMatrix();
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            EXPECT_FLOAT_EQ(matrix.m[row][col], Matrix4x4::Identity().m[row][col]);
        }
    }
}

TEST(TransformTest, DefaultAxesAreWorldAxes)
{
    const Transform transform;
    EXPECT_TRUE(NearlyEqual(transform.GetAxisX(), Vec3(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(NearlyEqual(transform.GetAxisY(), Vec3(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(NearlyEqual(transform.GetAxisZ(), Vec3(0.0f, 0.0f, 1.0f)));
}

TEST(TransformTest, RotatedAxesMatchQuaternionRotate)
{
    Transform transform;
    transform.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 90.0f);

    EXPECT_TRUE(NearlyEqual(transform.GetAxisX(), Vec3(0.0f, 0.0f, -1.0f), 1e-5f));
    EXPECT_TRUE(NearlyEqual(transform.GetAxisZ(), Vec3(1.0f, 0.0f, 0.0f), 1e-5f));
}

TEST(TransformTest, ToMatrixTransformPointMatchesPositionOffset)
{
    Transform transform;
    transform.position = Vec3(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(NearlyEqual(transform.ToMatrix().TransformPoint(Vec3::Zero()), transform.position));
}
