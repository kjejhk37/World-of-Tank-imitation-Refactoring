#include "platform/math/Matrix4x4.h"

#include <cmath>

#include "platform/math/MathConstants.h"

Matrix4x4 Matrix4x4::FromTRS(const Vec3& position, const Quaternion& rotation, const Vec3& scale)
{
    const float x = rotation.x;
    const float y = rotation.y;
    const float z = rotation.z;
    const float w = rotation.w;

    // 행 i = 단위 기저벡터 i가 회전 후 향하는 방향(Quaternion::Rotate와 일치하도록 유도된 식).
    const float rotation00 = 1.0f - 2.0f * (y * y + z * z);
    const float rotation01 = 2.0f * (x * y + w * z);
    const float rotation02 = 2.0f * (x * z - w * y);
    const float rotation10 = 2.0f * (x * y - w * z);
    const float rotation11 = 1.0f - 2.0f * (x * x + z * z);
    const float rotation12 = 2.0f * (y * z + w * x);
    const float rotation20 = 2.0f * (x * z + w * y);
    const float rotation21 = 2.0f * (y * z - w * x);
    const float rotation22 = 1.0f - 2.0f * (x * x + y * y);

    Matrix4x4 result;
    // 스케일은 행(row-vector 관례에서 입력 축 i에 해당) 단위로 곱한다.
    result.m[0][0] = rotation00 * scale.x;
    result.m[0][1] = rotation01 * scale.x;
    result.m[0][2] = rotation02 * scale.x;
    result.m[0][3] = 0.0f;

    result.m[1][0] = rotation10 * scale.y;
    result.m[1][1] = rotation11 * scale.y;
    result.m[1][2] = rotation12 * scale.y;
    result.m[1][3] = 0.0f;

    result.m[2][0] = rotation20 * scale.z;
    result.m[2][1] = rotation21 * scale.z;
    result.m[2][2] = rotation22 * scale.z;
    result.m[2][3] = 0.0f;

    result.m[3][0] = position.x;
    result.m[3][1] = position.y;
    result.m[3][2] = position.z;
    result.m[3][3] = 1.0f;

    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& rhs) const
{
    Matrix4x4 result;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                sum += m[row][k] * rhs.m[k][col];
            }
            result.m[row][col] = sum;
        }
    }
    return result;
}

Vec3 Matrix4x4::TransformPoint(const Vec3& point) const
{
    return Vec3(point.x * m[0][0] + point.y * m[1][0] + point.z * m[2][0] + m[3][0],
                point.x * m[0][1] + point.y * m[1][1] + point.z * m[2][1] + m[3][1],
                point.x * m[0][2] + point.y * m[1][2] + point.z * m[2][2] + m[3][2]);
}

Vec3 Matrix4x4::TransformDirection(const Vec3& direction) const
{
    return Vec3(direction.x * m[0][0] + direction.y * m[1][0] + direction.z * m[2][0],
                direction.x * m[0][1] + direction.y * m[1][1] + direction.z * m[2][1],
                direction.x * m[0][2] + direction.y * m[1][2] + direction.z * m[2][2]);
}

Matrix4x4 Matrix4x4::Transpose() const
{
    Matrix4x4 result;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            result.m[row][col] = m[col][row];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::InverseAffine() const
{
    const float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    const float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    const float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

    const float det = a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20);
    if (std::abs(det) < MathConstants::kEpsilon)
    {
        return Matrix4x4::Identity();
    }
    const float invDet = 1.0f / det;

    float inv[3][3];
    inv[0][0] = (a11 * a22 - a12 * a21) * invDet;
    inv[0][1] = (a02 * a21 - a01 * a22) * invDet;
    inv[0][2] = (a01 * a12 - a02 * a11) * invDet;
    inv[1][0] = (a12 * a20 - a10 * a22) * invDet;
    inv[1][1] = (a00 * a22 - a02 * a20) * invDet;
    inv[1][2] = (a02 * a10 - a00 * a12) * invDet;
    inv[2][0] = (a10 * a21 - a11 * a20) * invDet;
    inv[2][1] = (a01 * a20 - a00 * a21) * invDet;
    inv[2][2] = (a00 * a11 - a01 * a10) * invDet;

    Matrix4x4 result;
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            result.m[row][col] = inv[row][col];
        }
        result.m[row][3] = 0.0f;
    }

    const Vec3 translation(m[3][0], m[3][1], m[3][2]);
    result.m[3][0] = -(translation.x * inv[0][0] + translation.y * inv[1][0] + translation.z * inv[2][0]);
    result.m[3][1] = -(translation.x * inv[0][1] + translation.y * inv[1][1] + translation.z * inv[2][1]);
    result.m[3][2] = -(translation.x * inv[0][2] + translation.y * inv[1][2] + translation.z * inv[2][2]);
    result.m[3][3] = 1.0f;

    return result;
}
