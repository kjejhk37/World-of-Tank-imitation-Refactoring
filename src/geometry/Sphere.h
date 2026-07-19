#pragma once

#include "math/Vec3.h"

// Author: Claude
// Description: 구(Sphere). 중심점과 반지름으로 정의하는 순수 데이터 구조체.
// Input: 생성자 - center, radius
// Output: (해당 없음 - 데이터 구조체)
// Notes: 충돌/포함 판정은 이 클래스에 두지 않고 geometry/Intersections.h에 자유 함수로 모은다.
// Date: 2026-07-19
struct Sphere
{
    Vec3 center;
    float radius = 0.0f;

    Sphere() = default;
    Sphere(const Vec3& center, float radius) : center(center), radius(radius)
    {
    }
};
