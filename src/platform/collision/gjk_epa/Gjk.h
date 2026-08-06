#pragma once

#include <vector>

#include "platform/geometry/Geometry.h"
#include "platform/math/Vec3.h"

// Author: Claude
// Description: GJK(Gilbert-Johnson-Keerthi) 알고리즘 — 두 볼록 도형의 민코프스키 차 위에서 원점을
//              감싸는 심플렉스를 반복적으로 찾아 충돌 여부를 판정한다.
// Input: RunGjk(a, b) - 비교할 두 Geometry
// Output: GjkResult - intersects(충돌 여부) + simplex(충돌 시, EPA가 이어받을 4점 사면체)
// Notes: Geometry.h의 Support() 자유 함수만 사용한다(접근 퍼사드 규칙). simplex.points[0]이 가장 최근에
//        추가된 점이다(뒤로 갈수록 오래된 점) — DoSimplex 내부 케이스 처리가 이 순서를 전제한다.
// Date: 2026-07-20
struct GjkSimplex
{
    std::vector<Vec3> points;
};

struct GjkResult
{
    bool intersects = false;
    GjkSimplex simplex;
};

GjkResult RunGjk(const Geometry& a, const Geometry& b);
