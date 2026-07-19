#include <gtest/gtest.h>

#include "math/Matrix4x4.h"

TEST(Matrix4x4Test, IdentityTransformPointIsUnchanged)
{
    const Vec3 p(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(NearlyEqual(Matrix4x4::Identity().TransformPoint(p), p));
}

TEST(Matrix4x4Test, IdentityMultipliedByIdentityIsIdentity)
{
    const Matrix4x4 result = Matrix4x4::Identity() * Matrix4x4::Identity();
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            EXPECT_FLOAT_EQ(result.m[row][col], Matrix4x4::Identity().m[row][col]);
        }
    }
}

TEST(Matrix4x4Test, MultiplyingTwoNonTrivialMatricesMatchesSequentialTransformPoint)
{
    // 항등행렬끼리 곱하는 경우만으로는 곱셈 루프(행/열 인덱스 등)의 실제 정확성을 검증하지 못한다 —
    // 두 개의 비항등 TRS 행렬로 실제 곱셈 루프를 행사한다.
    // row-vector 관례에서 v*(A*B) == (v*A)*B == B.TransformPoint(A.TransformPoint(v)) 여야 한다.
    const Matrix4x4 a = Matrix4x4::FromTRS(Vec3(1.0f, 2.0f, 3.0f),
                                            Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 30.0f),
                                            Vec3(2.0f, 1.0f, 1.0f));
    const Matrix4x4 b = Matrix4x4::FromTRS(Vec3(-4.0f, 0.5f, 2.0f),
                                            Quaternion::FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), 60.0f),
                                            Vec3(1.0f, 3.0f, 0.5f));

    const Vec3 p(1.0f, -2.0f, 0.5f);
    const Vec3 viaMultiply = (a * b).TransformPoint(p);
    const Vec3 viaSequentialTransform = b.TransformPoint(a.TransformPoint(p));

    EXPECT_TRUE(NearlyEqual(viaMultiply, viaSequentialTransform, 1e-4f));
}

TEST(Matrix4x4Test, FromTRSTranslationOnlyMovesPoint)
{
    const Matrix4x4 m = Matrix4x4::FromTRS(Vec3(10.0f, 20.0f, 30.0f), Quaternion::Identity(), Vec3::One());
    EXPECT_TRUE(NearlyEqual(m.TransformPoint(Vec3(1.0f, 2.0f, 3.0f)), Vec3(11.0f, 22.0f, 33.0f)));
}

TEST(Matrix4x4Test, FromTRSScaleOnlyScalesPoint)
{
    const Matrix4x4 m = Matrix4x4::FromTRS(Vec3::Zero(), Quaternion::Identity(), Vec3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(NearlyEqual(m.TransformPoint(Vec3(1.0f, 1.0f, 1.0f)), Vec3(2.0f, 3.0f, 4.0f)));
}

TEST(Matrix4x4Test, TransformDirectionIgnoresTranslation)
{
    const Matrix4x4 m = Matrix4x4::FromTRS(Vec3(100.0f, 100.0f, 100.0f), Quaternion::Identity(), Vec3::One());
    EXPECT_TRUE(NearlyEqual(m.TransformDirection(Vec3(1.0f, 2.0f, 3.0f)), Vec3(1.0f, 2.0f, 3.0f)));
}

TEST(Matrix4x4Test, RotationBlockMatchesQuaternionRotate)
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.3f, 0.7f, 0.1f), 57.0f);
    const Matrix4x4 m = Matrix4x4::FromTRS(Vec3::Zero(), q, Vec3::One());

    const Vec3 probe(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(NearlyEqual(m.TransformDirection(probe), q.Rotate(probe), 1e-5f));
}

TEST(Matrix4x4Test, TransposeSwapsRowsAndColumns)
{
    Matrix4x4 m = Matrix4x4::Identity();
    m.m[0][1] = 5.0f;
    m.m[1][0] = 7.0f;

    const Matrix4x4 transposed = m.Transpose();
    EXPECT_FLOAT_EQ(transposed.m[1][0], 5.0f);
    EXPECT_FLOAT_EQ(transposed.m[0][1], 7.0f);
}

TEST(Matrix4x4Test, InverseAffineRoundTripsToOriginalPoint)
{
    const Matrix4x4 m = Matrix4x4::FromTRS(Vec3(5.0f, -3.0f, 2.0f),
                                            Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 40.0f),
                                            Vec3(2.0f, 1.0f, 0.5f));
    const Matrix4x4 inverse = m.InverseAffine();

    const Vec3 original(1.0f, 2.0f, 3.0f);
    const Vec3 transformed = m.TransformPoint(original);
    const Vec3 restored = inverse.TransformPoint(transformed);

    EXPECT_TRUE(NearlyEqual(restored, original, 1e-4f));
}

TEST(Matrix4x4Test, InverseAffineOfSingularMatrixReturnsIdentity)
{
    Matrix4x4 singular = Matrix4x4::Identity();
    singular.m[0][0] = 0.0f;
    singular.m[1][1] = 0.0f;
    singular.m[2][2] = 0.0f;

    const Matrix4x4 inverse = singular.InverseAffine();
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            EXPECT_FLOAT_EQ(inverse.m[row][col], Matrix4x4::Identity().m[row][col]);
        }
    }
}
