#pragma once

#include "collision/gjk_epa/Gjk.h"
#include "geometry/Geometry.h"
#include "math/Vec3.h"

// Author: Claude
// Description: EPA(Expanding Polytope Algorithm) — GJK가 반환한 원점을 감싸는 사면체를 초기 폴리토프로
//              확장해, 두 도형의 침투 법선과 깊이를 계산한다.
// Input: RunEpa(a, b, startingSimplex) - 비교할 두 Geometry + RunGjk()가 반환한 4점 사면체
// Output: PenetrationInfo - 침투 법선(단위 벡터, A에서 B를 미는 방향) + 깊이(항상 >= 0)
// Notes: startingSimplex는 반드시 GjkResult::intersects == true일 때의 4점 사면체여야 한다(호출자 책임).
// Date: 2026-07-20
struct PenetrationInfo
{
    Vec3 normal;
    float depth = 0.0f;
};

PenetrationInfo RunEpa(const Geometry& a, const Geometry& b, const GjkSimplex& startingSimplex);
