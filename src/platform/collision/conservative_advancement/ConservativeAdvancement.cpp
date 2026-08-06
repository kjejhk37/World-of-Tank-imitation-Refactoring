#include "platform/collision/conservative_advancement/ConservativeAdvancement.h"

#include "platform/math/MathConstants.h"

// Author: Claude
// Description: ConservativeAdvancement()의 구현 — 경계 구 기반 거리/접근 속도로 TOI를 반복적으로 좁힌다.
// Input/Output: (ConservativeAdvancement.h 참고)
// Date: 2026-07-20

ToiResult ConservativeAdvancement(const Geometry& a, const Vec3& velocityA, const Geometry& b, const Vec3& velocityB,
                                   float deltaTime)
{
    const AABB boundsA = GetBounds(a);
    const AABB boundsB = GetBounds(b);
    const float radiusA = boundsA.Extents().Length();
    const float radiusB = boundsB.Extents().Length();
    const float combinedRadius = radiusA + radiusB;
    const Vec3 centerA0 = boundsA.Center();
    const Vec3 centerB0 = boundsB.Center();

    constexpr float kDistanceTolerance = 1e-3f;
    constexpr int kMaxIterations = 32;

    float t = 0.0f;
    for (int i = 0; i < kMaxIterations; ++i)
    {
        const Vec3 centerA = centerA0 + velocityA * t;
        const Vec3 centerB = centerB0 + velocityB * t;
        const Vec3 offset = centerB - centerA;
        const float centerDistance = offset.Length();
        const float distance = centerDistance - combinedRadius;

        if (distance < kDistanceTolerance)
        {
            return ToiResult{true, t};
        }

        // centerDistance > combinedRadius >= 0이 위에서 이미 보장되므로 여기서는 항상 0보다 크다.
        const Vec3 direction = offset / centerDistance;
        const float closingSpeed = -Dot(direction, velocityB - velocityA);

        if (closingSpeed <= MathConstants::kEpsilon)
        {
            return ToiResult{false, deltaTime};
        }

        t += distance / closingSpeed;
        if (t >= deltaTime)
        {
            return ToiResult{false, deltaTime};
        }
    }

    // 반복 상한에 도달 — 거리가 tolerance 이하로 수렴했는지 확인되지 않았으므로, GJK의 비수렴 기본값과
    // 마찬가지로 "확정 못하면 충돌 아님"을 안전한 기본값으로 반환한다(거짓 양성보다 거짓 음성이 안전).
    return ToiResult{false, deltaTime};
}
