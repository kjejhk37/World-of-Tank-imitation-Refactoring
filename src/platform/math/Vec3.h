#pragma once

#include <cmath>

#include "platform/math/MathConstants.h"

// Author: Claude
// Description: 3차원 벡터. 덧셈/뺄셈/스칼라 및 컴포넌트별 곱셈/나눗셈/외적/내적/길이/정규화를 제공한다.
// Input: 생성자 - x, y, z (기본값 0,0,0)
// Output: (해당 없음 - 데이터 구조체 + 연산)
// Notes: 이 파일은 프로젝트의 다른 모듈을 전혀 참조하지 않는다(math/geometry의 향후 submodule 분리 전제).
//        연산이 전부 몇 줄이라 헤더 온리(인라인)로 유지한다.
//        operator==/!=는 정확 비교다 — 경계/충돌 판정처럼 오차 허용이 필요한 곳은 NearlyEqual을 쓴다.
// Date: 2026-07-19
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3() = default;
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z)
    {
    }

    static constexpr Vec3 Zero()
    {
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    static constexpr Vec3 One()
    {
        return Vec3(1.0f, 1.0f, 1.0f);
    }

    constexpr Vec3 operator+(const Vec3& rhs) const
    {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    constexpr Vec3 operator-(const Vec3& rhs) const
    {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    constexpr Vec3 operator-() const
    {
        return Vec3(-x, -y, -z);
    }

    constexpr Vec3 operator*(float scalar) const
    {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    constexpr Vec3 operator/(float scalar) const
    {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    // 컴포넌트별(Hadamard) 곱 — 내적은 별도 자유 함수 Dot()로 제공한다.
    constexpr Vec3 operator*(const Vec3& rhs) const
    {
        return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
    }

    Vec3& operator+=(const Vec3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    Vec3& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3& operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // 인덱스 기반 축 접근 — SAT 등 축을 순회하는 충돌 판정 코드에서 사용한다. 0=x, 1=y, 그 외=z.
    constexpr float operator[](int index) const
    {
        return index == 0 ? x : (index == 1 ? y : z);
    }

    constexpr bool operator==(const Vec3& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    constexpr bool operator!=(const Vec3& rhs) const
    {
        return !(*this == rhs);
    }

    constexpr float LengthSquared() const
    {
        return x * x + y * y + z * z;
    }

    float Length() const
    {
        return std::sqrt(LengthSquared());
    }

    // 길이가 kEpsilon 미만이면(영벡터에 가까우면) 0 나눗셈 대신 영벡터를 반환한다.
    Vec3 Normalized() const
    {
        const float length = Length();
        if (length < MathConstants::kEpsilon)
        {
            return Vec3::Zero();
        }
        return *this / length;
    }
};

constexpr Vec3 operator*(float scalar, const Vec3& v)
{
    return v * scalar;
}

constexpr float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline bool NearlyEqual(const Vec3& a, const Vec3& b, float epsilon = MathConstants::kEpsilon)
{
    return std::abs(a.x - b.x) <= epsilon && std::abs(a.y - b.y) <= epsilon && std::abs(a.z - b.z) <= epsilon;
}
