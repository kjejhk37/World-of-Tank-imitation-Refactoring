#include <gtest/gtest.h>

#include "geometry/OBB.h"

TEST(OBBTest, DefaultOrientationAxesAreWorldAxes)
{
    const OBB box(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(NearlyEqual(box.GetAxis(0), Vec3(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(NearlyEqual(box.GetAxis(1), Vec3(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(NearlyEqual(box.GetAxis(2), Vec3(0.0f, 0.0f, 1.0f)));
}

TEST(OBBTest, RotatedOrientationAxesMatchTransformRotation)
{
    Transform transform;
    transform.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 90.0f);
    const OBB box(transform, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_TRUE(NearlyEqual(box.GetAxis(0), Vec3(0.0f, 0.0f, -1.0f), 1e-5f));
    EXPECT_TRUE(NearlyEqual(box.GetAxis(2), Vec3(1.0f, 0.0f, 0.0f), 1e-5f));
}
