#include "collision/mpr/Mpr.h"

#include "math/MathConstants.h"

// Author: Claude
// Description: MprIntersects()의 구현 — XenoCollide 방식 포탈 탐색 + 정제.
// Input/Output: (Mpr.h 참고)
// Notes: v0(내부 기준점)는 GJK의 초기 탐색 방향과 마찬가지로 GetBounds() 중심 차로 근사한다.
// Date: 2026-07-20

namespace
{
    Vec3 SupportM(const Geometry& a, const Geometry& b, const Vec3& direction)
    {
        return Support(a, direction) - Support(b, -direction);
    }
}

bool MprIntersects(const Geometry& a, const Geometry& b)
{
    const Vec3 v0 = GetBounds(a).Center() - GetBounds(b).Center();
    if (v0.LengthSquared() < MathConstants::kEpsilon * MathConstants::kEpsilon)
    {
        return true;  // 내부 기준점이 원점과 일치 — 사실상 충돌로 간주
    }

    const Vec3 d = -v0;  // v0에서 원점을 향하는 방향

    Vec3 v1 = SupportM(a, b, d);
    if (Dot(v1, d) <= 0.0f)
    {
        return false;
    }

    Vec3 n = Cross(v1 - v0, d);
    if (n.LengthSquared() < MathConstants::kEpsilon * MathConstants::kEpsilon)
    {
        n = Cross(v1 - v0, Vec3(1.0f, 0.0f, 0.0f));
        if (n.LengthSquared() < MathConstants::kEpsilon * MathConstants::kEpsilon)
        {
            n = Cross(v1 - v0, Vec3(0.0f, 1.0f, 0.0f));
        }
    }
    Vec3 v2 = SupportM(a, b, n);
    if (Dot(v2, n) <= 0.0f)
    {
        return false;
    }

    // 포탈이 광선 d를 각도상 감싸도록(원점 방향이 v1-v0-v2 사이에 오도록) 보정.
    constexpr int kMaxBracketIterations = 32;
    for (int i = 0; i < kMaxBracketIterations; ++i)
    {
        const Vec3 edge1Normal = Cross(v1 - v0, d);
        if (Dot(edge1Normal, v2 - v0) < 0.0f)
        {
            v2 = SupportM(a, b, edge1Normal);
            if (Dot(v2, edge1Normal) <= 0.0f)
            {
                return false;
            }
            continue;
        }
        const Vec3 edge2Normal = Cross(d, v2 - v0);
        if (Dot(edge2Normal, v1 - v0) < 0.0f)
        {
            v1 = SupportM(a, b, edge2Normal);
            if (Dot(v1, edge2Normal) <= 0.0f)
            {
                return false;
            }
            continue;
        }
        break;
    }

    // 포탈을 경계 쪽으로 반복 확장하며 정제. v1/v2의 역할(브라켓 단계에서 정한 "어느 쪽 변인지")은 절대
    // 서로 바꾸지 않는다 — 법선이 바깥쪽(v0 반대편)을 향하도록 부호만 뒤집어서 쓴다. v1/v2를 물리적으로
    // swap하면 "v3가 v1쪽을 대체하는지 v2쪽을 대체하는지" 판정의 기준이 매 반복 흔들려 수렴하지 않는다.
    constexpr int kMaxRefineIterations = 32;
    constexpr float kTolerance = 1e-4f;
    for (int i = 0; i < kMaxRefineIterations; ++i)
    {
        const Vec3 rawNormal = Cross(v1 - v0, v2 - v0);
        const Vec3 outwardNormal = (Dot(rawNormal, v0) > 0.0f) ? -rawNormal : rawNormal;

        const Vec3 v3 = SupportM(a, b, outwardNormal);
        if (Dot(v3, outwardNormal) - Dot(v0, outwardNormal) < kTolerance)
        {
            return Dot(outwardNormal, v1) >= 0.0f;
        }

        // v3를 새 정점으로 넣었을 때 (v0,v1,v3) 포탈이 여전히 광선 d를 감싸면 v2를 v3로 교체하고,
        // 그렇지 않으면((v0,v3,v2) 쪽이 감싼다는 것이 불변식) v1을 v3로 교체한다 — 브라켓 단계와 동일한
        // 판정식을 재사용해 일관되게 판단한다.
        const Vec3 e1 = Cross(v1 - v0, d);
        const Vec3 e2 = Cross(d, v3 - v0);
        const bool keepsV1Side = Dot(e1, v3 - v0) >= 0.0f && Dot(e2, v1 - v0) >= 0.0f;
        if (keepsV1Side)
        {
            v2 = v3;
        }
        else
        {
            v1 = v3;
        }
    }

    const Vec3 finalNormal = Cross(v1 - v0, v2 - v0);
    const Vec3 outwardFinal = (Dot(finalNormal, v0) > 0.0f) ? -finalNormal : finalNormal;
    return Dot(outwardFinal, v1) >= 0.0f;
}
