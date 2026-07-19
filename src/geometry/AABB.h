#pragma once

#include "math/Vec3.h"

// Author: Claude
// Description: 축 정렬 경계 상자(Axis-Aligned Bounding Box). min/max 두 점으로 정의한다.
// Input: 생성자 - min, max (각 축에서 min은 max 이하여야 한다 — 호출자 책임, 별도 검증 없음)
// Output: Center() - 중심점 / Extents() - 각 축 half-size
// Notes: 충돌/포함 판정은 이 클래스에 두지 않고 geometry/Intersections.h에 자유 함수로 모은다
//        (여러 도형 쌍을 한 곳에서 일관되게 관리하기 위함 — 전략 문서 참고).
// Date: 2026-07-19
struct AABB
{
    Vec3 min;
    Vec3 max;

    AABB() = default;
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max)
    {
    }

    Vec3 Center() const
    {
        return (min + max) * 0.5f;
    }

    Vec3 Extents() const
    {
        return (max - min) * 0.5f;
    }
};
