#pragma once

#include <cmath>

#include "platform/math/MathConstants.h"
#include "platform/math/Vec3.h"

// Author: Claude
// Description: 회전을 표현하는 단위 사원수(quaternion). 축-각/오일러 각으로부터 생성, 회전 합성,
//              벡터 회전을 제공한다.
// Input: 생성자 - x, y, z, w (기본값은 단위 회전) / FromAxisAngle - 축, 각도(Degree) /
//        FromEulerDegrees - pitch(X축), yaw(Y축), roll(Z축), 전부 Degree
// Output: (해당 없음 - 데이터 구조체 + 연산)
// Notes: 공개 API의 각도 파라미터는 전부 Degree다 — 내부적으로만 MathConstants::DegreesToRadians로
//        라디안 변환해 sin/cos에 사용한다.
//        operator*(a, b)는 "a 다음에 b를 적용"이 아니라 사원수 합성의 표준 관례를 따른다 —
//        (a*b).Rotate(v) == a.Rotate(b.Rotate(v)), 즉 벡터에는 b가 먼저 적용되고 a가 나중에 적용된다.
//        FromEulerDegrees는 qYaw*qPitch*qRoll 순으로 합성하므로, 벡터에 적용될 때는
//        Roll(Z) → Pitch(X) → Yaw(Y) 순서로 적용된다.
//        Rotate()는 *this가 단위 사원수(길이 1)라고 가정한다 — FromAxisAngle/FromEulerDegrees/
//        Normalized()가 만드는 값은 이 가정을 만족한다.
// Date: 2026-07-19
struct Quaternion
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
    {
    }

    static constexpr Quaternion Identity()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Quaternion FromAxisAngle(const Vec3& axis, float angleDegrees)
    {
        const Vec3 normalizedAxis = axis.Normalized();
        const float halfAngleRadians = MathConstants::DegreesToRadians(angleDegrees) * 0.5f;
        const float s = std::sin(halfAngleRadians);
        return Quaternion(normalizedAxis.x * s, normalizedAxis.y * s, normalizedAxis.z * s,
                           std::cos(halfAngleRadians));
    }

    // 합성 순서: qYaw * qPitch * qRoll — 벡터에는 Roll → Pitch → Yaw 순으로 적용된다(클래스 Notes 참고).
    static Quaternion FromEulerDegrees(float pitchDegrees, float yawDegrees, float rollDegrees)
    {
        const Quaternion yaw = FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), yawDegrees);
        const Quaternion pitch = FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), pitchDegrees);
        const Quaternion roll = FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), rollDegrees);
        return yaw * pitch * roll;
    }

    constexpr Quaternion operator*(const Quaternion& rhs) const
    {
        return Quaternion(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
                           w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
                           w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
                           w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    Quaternion Normalized() const
    {
        const float length = Length();
        if (length < MathConstants::kEpsilon)
        {
            return Quaternion::Identity();
        }
        return Quaternion(x / length, y / length, z / length, w / length);
    }

    constexpr Quaternion Conjugate() const
    {
        return Quaternion(-x, -y, -z, w);
    }

    Quaternion Inverse() const
    {
        const float lengthSquared = x * x + y * y + z * z + w * w;
        if (lengthSquared < MathConstants::kEpsilon)
        {
            return Quaternion::Identity();
        }
        const Quaternion conjugate = Conjugate();
        return Quaternion(conjugate.x / lengthSquared, conjugate.y / lengthSquared, conjugate.z / lengthSquared,
                           conjugate.w / lengthSquared);
    }

    // 최적화된 공식: v' = v + 2*w*(qv x v) + 2*(qv x (qv x v)), qv = (x,y,z).
    Vec3 Rotate(const Vec3& v) const
    {
        const Vec3 qv(x, y, z);
        const Vec3 t = 2.0f * Cross(qv, v);
        return v + w * t + Cross(qv, t);
    }
};
