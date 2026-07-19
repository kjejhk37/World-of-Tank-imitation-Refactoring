#include <gtest/gtest.h>

#include "math/Vec3.h"

TEST(Vec3Test, Addition)
{
    const Vec3 result = Vec3(1.0f, 2.0f, 3.0f) + Vec3(4.0f, 5.0f, 6.0f);
    EXPECT_EQ(result, Vec3(5.0f, 7.0f, 9.0f));
}

TEST(Vec3Test, Subtraction)
{
    const Vec3 result = Vec3(4.0f, 5.0f, 6.0f) - Vec3(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(result, Vec3(3.0f, 3.0f, 3.0f));
}

TEST(Vec3Test, UnaryNegation)
{
    EXPECT_EQ(-Vec3(1.0f, -2.0f, 3.0f), Vec3(-1.0f, 2.0f, -3.0f));
}

TEST(Vec3Test, ScalarMultiplicationAndDivision)
{
    EXPECT_EQ(Vec3(1.0f, 2.0f, 3.0f) * 2.0f, Vec3(2.0f, 4.0f, 6.0f));
    EXPECT_EQ(2.0f * Vec3(1.0f, 2.0f, 3.0f), Vec3(2.0f, 4.0f, 6.0f));
    EXPECT_EQ(Vec3(2.0f, 4.0f, 6.0f) / 2.0f, Vec3(1.0f, 2.0f, 3.0f));
}

TEST(Vec3Test, ComponentWiseMultiplication)
{
    EXPECT_EQ(Vec3(1.0f, 2.0f, 3.0f) * Vec3(4.0f, 5.0f, 6.0f), Vec3(4.0f, 10.0f, 18.0f));
}

TEST(Vec3Test, CompoundAssignmentOperators)
{
    Vec3 v(1.0f, 2.0f, 3.0f);
    v += Vec3(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(v, Vec3(2.0f, 3.0f, 4.0f));

    v -= Vec3(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(v, Vec3(1.0f, 2.0f, 3.0f));

    v *= 2.0f;
    EXPECT_EQ(v, Vec3(2.0f, 4.0f, 6.0f));

    v /= 2.0f;
    EXPECT_EQ(v, Vec3(1.0f, 2.0f, 3.0f));
}

TEST(Vec3Test, IndexOperatorAccessesComponentsInOrder)
{
    const Vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);
}

TEST(Vec3Test, DotProductOfOrthogonalVectorsIsZero)
{
    EXPECT_FLOAT_EQ(Dot(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f)), 0.0f);
}

TEST(Vec3Test, DotProductOfParallelVectorsMatchesLengthProduct)
{
    EXPECT_FLOAT_EQ(Dot(Vec3(2.0f, 0.0f, 0.0f), Vec3(3.0f, 0.0f, 0.0f)), 6.0f);
}

TEST(Vec3Test, CrossProductOfXAndYAxisIsZAxis)
{
    EXPECT_EQ(Cross(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f)), Vec3(0.0f, 0.0f, 1.0f));
}

TEST(Vec3Test, CrossProductOfParallelVectorsIsZero)
{
    EXPECT_EQ(Cross(Vec3(2.0f, 0.0f, 0.0f), Vec3(4.0f, 0.0f, 0.0f)), Vec3::Zero());
}

TEST(Vec3Test, LengthAndLengthSquared)
{
    const Vec3 v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.LengthSquared(), 25.0f);
    EXPECT_FLOAT_EQ(v.Length(), 5.0f);
}

TEST(Vec3Test, NormalizedProducesUnitLength)
{
    const Vec3 normalized = Vec3(3.0f, 4.0f, 0.0f).Normalized();
    EXPECT_NEAR(normalized.Length(), 1.0f, 1e-6f);
    EXPECT_TRUE(NearlyEqual(normalized, Vec3(0.6f, 0.8f, 0.0f)));
}

TEST(Vec3Test, NormalizedOfZeroVectorReturnsZeroWithoutCrashing)
{
    EXPECT_EQ(Vec3::Zero().Normalized(), Vec3::Zero());
}

TEST(Vec3Test, NearlyEqualToleratesSmallDifferences)
{
    EXPECT_TRUE(NearlyEqual(Vec3(1.0f, 1.0f, 1.0f), Vec3(1.0f + 1e-7f, 1.0f, 1.0f)));
    EXPECT_FALSE(NearlyEqual(Vec3(1.0f, 1.0f, 1.0f), Vec3(1.1f, 1.0f, 1.0f)));
}
