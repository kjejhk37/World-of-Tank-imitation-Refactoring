#pragma once

#include "math/Vec3.h"

// Author: Claude
// Description: 원기둥(Cylinder). 두 끝점(축 선분)과 반지름으로 정의한다 — 양 끝이 평평한 원판으로
//              막힌 도형. Capsule과 데이터 표현은 같고 캡의 형태(평평 vs 둥근)만 다르다.
// Input: 생성자 - pointA, pointB(축 선분 양 끝), radius
// Output: (해당 없음 - 데이터 구조체)
// Notes: 충돌/포함 판정은 이 클래스에 두지 않고 geometry/Intersections.h에 자유 함수로 모은다.
//        Cylinder가 낀 일부 쌍(Cylinder-Cylinder 등)은 이번 사이클에서 [DEFERRED] — 전략 문서 참고.
// Date: 2026-07-19
struct Cylinder
{
    Vec3 pointA;
    Vec3 pointB;
    float radius = 0.0f;

    Cylinder() = default;
    Cylinder(const Vec3& pointA, const Vec3& pointB, float radius) : pointA(pointA), pointB(pointB), radius(radius)
    {
    }
};
