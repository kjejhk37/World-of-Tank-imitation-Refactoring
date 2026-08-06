#pragma once

#include "platform/math/Vec3.h"

// Author: Claude
// Description: 캡슐(Capsule). 두 끝점(중심 선분)과 반지름으로 정의한다 — 양 끝이 반구(hemisphere)로
//              막힌 도형. Cylinder와 데이터 표현은 같고 캡의 형태(둥근 vs 평평)만 다르다.
// Input: 생성자 - pointA, pointB(선분 양 끝), radius
// Output: (해당 없음 - 데이터 구조체)
// Notes: 충돌/포함 판정은 이 클래스에 두지 않고 geometry/Intersections.h에 자유 함수로 모은다.
// Date: 2026-07-19
struct Capsule
{
    Vec3 pointA;
    Vec3 pointB;
    float radius = 0.0f;

    Capsule() = default;
    Capsule(const Vec3& pointA, const Vec3& pointB, float radius) : pointA(pointA), pointB(pointB), radius(radius)
    {
    }
};
