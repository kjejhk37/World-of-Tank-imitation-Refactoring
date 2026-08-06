#include <gtest/gtest.h>

#include "platform/math/Quaternion.h"

TEST(QuaternionTest, IdentityDoesNotChangeVector)
{
    const Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(NearlyEqual(Quaternion::Identity().Rotate(v), v));
}

TEST(QuaternionTest, RotateAroundYAxisBy90DegreesMatchesRightHandRule)
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 90.0f);
    const Vec3 rotated = q.Rotate(Vec3(1.0f, 0.0f, 0.0f));
    EXPECT_TRUE(NearlyEqual(rotated, Vec3(0.0f, 0.0f, -1.0f), 1e-5f));
}

TEST(QuaternionTest, RotateAroundZAxisBy180DegreesFlipsXAxis)
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), 180.0f);
    const Vec3 rotated = q.Rotate(Vec3(1.0f, 0.0f, 0.0f));
    EXPECT_TRUE(NearlyEqual(rotated, Vec3(-1.0f, 0.0f, 0.0f), 1e-5f));
}

TEST(QuaternionTest, FromEulerDegreesWithOnlyPitchMatchesAxisAngle)
{
    const Quaternion euler = Quaternion::FromEulerDegrees(37.0f, 0.0f, 0.0f);
    const Quaternion axisAngle = Quaternion::FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), 37.0f);
    const Vec3 probe(0.0f, 1.0f, 1.0f);
    EXPECT_TRUE(NearlyEqual(euler.Rotate(probe), axisAngle.Rotate(probe), 1e-5f));
}

TEST(QuaternionTest, FromEulerDegreesWithOnlyYawMatchesAxisAngle)
{
    const Quaternion euler = Quaternion::FromEulerDegrees(0.0f, 52.0f, 0.0f);
    const Quaternion axisAngle = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 52.0f);
    const Vec3 probe(1.0f, 0.0f, 1.0f);
    EXPECT_TRUE(NearlyEqual(euler.Rotate(probe), axisAngle.Rotate(probe), 1e-5f));
}

TEST(QuaternionTest, FromEulerDegreesWithOnlyRollMatchesAxisAngle)
{
    const Quaternion euler = Quaternion::FromEulerDegrees(0.0f, 0.0f, 64.0f);
    const Quaternion axisAngle = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), 64.0f);
    const Vec3 probe(1.0f, 1.0f, 0.0f);
    EXPECT_TRUE(NearlyEqual(euler.Rotate(probe), axisAngle.Rotate(probe), 1e-5f));
}

TEST(QuaternionTest, FromEulerDegreesComposesRollThenPitchThenYaw)
{
    const Quaternion euler = Quaternion::FromEulerDegrees(30.0f, 45.0f, 60.0f);

    const Quaternion yaw = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 45.0f);
    const Quaternion pitch = Quaternion::FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), 30.0f);
    const Quaternion roll = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), 60.0f);

    const Vec3 probe(1.0f, 2.0f, 3.0f);
    const Vec3 expected = yaw.Rotate(pitch.Rotate(roll.Rotate(probe)));

    EXPECT_TRUE(NearlyEqual(euler.Rotate(probe), expected, 1e-5f));
}

TEST(QuaternionTest, CompositionMatchesApplyingRightOperandFirst)
{
    const Quaternion a = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 90.0f);
    const Quaternion b = Quaternion::FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), 45.0f);
    const Vec3 probe(1.0f, 1.0f, 1.0f);

    const Vec3 viaComposition = (a * b).Rotate(probe);
    const Vec3 viaSequentialRotate = a.Rotate(b.Rotate(probe));

    EXPECT_TRUE(NearlyEqual(viaComposition, viaSequentialRotate, 1e-5f));
}

TEST(QuaternionTest, InverseUndoesRotation)
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.3f, 0.7f, 0.1f), 73.0f);
    const Vec3 probe(2.0f, -1.0f, 4.0f);

    const Vec3 rotated = q.Rotate(probe);
    const Vec3 restored = q.Inverse().Rotate(rotated);

    EXPECT_TRUE(NearlyEqual(restored, probe, 1e-5f));
}

TEST(QuaternionTest, NormalizedProducesUnitLength)
{
    const Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_NEAR(q.Normalized().Length(), 1.0f, 1e-6f);
}
