#include "platform/collision/gjk_epa/Gjk.h"

#include "platform/collision/gjk_epa/MinkowskiSupport.h"
#include "platform/math/MathConstants.h"

// Author: Claude
// Description: RunGjk()의 구현 — 표준 Voronoi-region 기반 GJK(점/선/삼각형/사면체 케이스).
// Input/Output: (Gjk.h 참고)
// Notes: simplex는 항상 [가장 최근에 추가된 점, ..., 가장 오래된 점] 순서로 유지한다.
// Date: 2026-07-20

namespace
{
    // 축 정렬된 대칭 배치(예: 두 구가 한 축 위에 정확히 나란함)에서는 GJK 탐색 방향이 도형의 대칭축과
    // 정확히 일치해 심플렉스 점들이 정확한 대척점 쌍이 되고, 이는 EPA에서 원점이 여러 면의 평면 위에
    // 정확히 걸치는 퇴화 케이스로 이어진다. 임의로 조정/삭제해도 되는 값이 아니라 이 대칭을 의도적으로
    // 깨뜨리기 위한 고정값이다 — 세 성분이 서로 다르고 어떤 축에도 정렬되지 않아야 한다.
    const Vec3 kInitialDirectionJitter(0.3f, 0.4f, 0.35f);

    bool SameDirection(const Vec3& direction, const Vec3& ao)
    {
        return Dot(direction, ao) > 0.0f;
    }

    // ab와 ao가 (거의) 평행할 때는 Cross(ab, ao)가 영벡터가 되어 통상의 삼중곱 공식이 무너진다
    // (예: 두 도형의 중심이 한 축 위에 정확히 나란한 대칭 배치). 이 경우 ab에 평행하지 않은 임의의
    // 월드 축을 골라 수직 방향을 강제로 만들어 탐색이 그 축 방향으로 계속 진행되게 한다.
    Vec3 PerpendicularTowardOrigin(const Vec3& ab, const Vec3& ao)
    {
        const Vec3 cross = Cross(ab, ao);
        if (cross.LengthSquared() < MathConstants::kEpsilon * MathConstants::kEpsilon)
        {
            const Vec3 abNormalized = ab.Normalized();
            // 순수 월드 축 대신 축에 치우치지 않은 방향을 후보로 써서, 이후 단계에서도 축 정렬 대칭이
            // 반복적으로 재발하지 않게 한다(위 RunGjk의 지터와 같은 목적).
            const Vec3 fallbackAxis = std::abs(Dot(abNormalized, Vec3(1.0f, 0.0f, 0.0f))) < 0.9f
                                           ? Vec3(1.0f, 0.0f, 0.0f)
                                           : Vec3(0.0f, 1.0f, 0.05f);
            return Cross(Cross(ab, fallbackAxis), ab);
        }
        return Cross(cross, ab);
    }

    bool LineCase(std::vector<Vec3>& simplex, Vec3& direction)
    {
        const Vec3 a = simplex[0];
        const Vec3 b = simplex[1];
        const Vec3 ab = b - a;
        const Vec3 ao = -a;

        if (SameDirection(ab, ao))
        {
            direction = PerpendicularTowardOrigin(ab, ao);
        }
        else
        {
            simplex = {a};
            direction = ao;
        }
        return false;
    }

    bool TriangleCase(std::vector<Vec3>& simplex, Vec3& direction)
    {
        const Vec3 a = simplex[0];
        const Vec3 b = simplex[1];
        const Vec3 c = simplex[2];
        const Vec3 ab = b - a;
        const Vec3 ac = c - a;
        const Vec3 ao = -a;
        const Vec3 abc = Cross(ab, ac);

        if (SameDirection(Cross(abc, ac), ao))
        {
            if (SameDirection(ac, ao))
            {
                simplex = {a, c};
                direction = Cross(Cross(ac, ao), ac);
                return false;
            }
            simplex = {a, b};
            return LineCase(simplex, direction);
        }

        if (SameDirection(Cross(ab, abc), ao))
        {
            simplex = {a, b};
            return LineCase(simplex, direction);
        }

        if (SameDirection(abc, ao))
        {
            direction = abc;
        }
        else
        {
            simplex = {a, c, b};
            direction = -abc;
        }
        return false;
    }

    bool TetrahedronCase(std::vector<Vec3>& simplex, Vec3& direction)
    {
        const Vec3 a = simplex[0];
        const Vec3 b = simplex[1];
        const Vec3 c = simplex[2];
        const Vec3 d = simplex[3];
        const Vec3 ab = b - a;
        const Vec3 ac = c - a;
        const Vec3 ad = d - a;
        const Vec3 ao = -a;

        const Vec3 abc = Cross(ab, ac);
        const Vec3 acd = Cross(ac, ad);
        const Vec3 adb = Cross(ad, ab);

        if (SameDirection(abc, ao))
        {
            simplex = {a, b, c};
            return TriangleCase(simplex, direction);
        }
        if (SameDirection(acd, ao))
        {
            simplex = {a, c, d};
            return TriangleCase(simplex, direction);
        }
        if (SameDirection(adb, ao))
        {
            simplex = {a, d, b};
            return TriangleCase(simplex, direction);
        }
        return true;
    }

    bool DoSimplex(std::vector<Vec3>& simplex, Vec3& direction)
    {
        switch (simplex.size())
        {
        case 2:
            return LineCase(simplex, direction);
        case 3:
            return TriangleCase(simplex, direction);
        case 4:
            return TetrahedronCase(simplex, direction);
        default:
            return false;
        }
    }
}

GjkResult RunGjk(const Geometry& a, const Geometry& b)
{
    Vec3 direction = GetBounds(b).Center() - GetBounds(a).Center() + kInitialDirectionJitter;
    if (direction.LengthSquared() < MathConstants::kEpsilon * MathConstants::kEpsilon)
    {
        direction = Vec3(1.0f, 0.0f, 0.0f);
    }

    std::vector<Vec3> simplex;
    simplex.push_back(MinkowskiSupport(a, b, direction));
    direction = -simplex[0];

    constexpr int kMaxIterations = 64;
    for (int i = 0; i < kMaxIterations; ++i)
    {
        const Vec3 newPoint = MinkowskiSupport(a, b, direction);
        if (Dot(newPoint, direction) < 0.0f)
        {
            return GjkResult{false, {}};
        }

        simplex.insert(simplex.begin(), newPoint);
        if (DoSimplex(simplex, direction))
        {
            return GjkResult{true, GjkSimplex{simplex}};
        }
    }
    return GjkResult{false, {}};
}
