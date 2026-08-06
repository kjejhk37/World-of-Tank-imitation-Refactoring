#include "platform/collision/gjk_epa/GjkEpa.h"

#include "platform/collision/gjk_epa/Gjk.h"

// Author: Claude
// Description: Intersects()/ComputePenetration()의 구현 — RunGjk()/RunEpa()를 엮는 얇은 퍼사드.
// Input/Output: (GjkEpa.h 참고)
// Date: 2026-07-20

bool Intersects(const Geometry& a, const Geometry& b)
{
    return RunGjk(a, b).intersects;
}

std::optional<PenetrationInfo> ComputePenetration(const Geometry& a, const Geometry& b)
{
    const GjkResult gjk = RunGjk(a, b);
    if (!gjk.intersects)
    {
        return std::nullopt;
    }
    return RunEpa(a, b, gjk.simplex);
}
