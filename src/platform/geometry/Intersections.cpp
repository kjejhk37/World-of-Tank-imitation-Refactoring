#include "platform/geometry/Intersections.h"

#include <algorithm>
#include <cmath>

#include "platform/math/MathConstants.h"

namespace
{
    using MathConstants::kEpsilon;

    Vec3 ClosestPointOnAABB(const Vec3& point, const AABB& box)
    {
        return Vec3(std::clamp(point.x, box.min.x, box.max.x), std::clamp(point.y, box.min.y, box.max.y),
                    std::clamp(point.z, box.min.z, box.max.z));
    }

    Vec3 ClosestPointOnOBB(const Vec3& point, const OBB& box)
    {
        const Vec3 d = point - box.transform.position;
        Vec3 closest = box.transform.position;
        for (int i = 0; i < 3; ++i)
        {
            const Vec3 axis = box.GetAxis(i);
            const float extent = box.halfExtents[i];
            const float distance = std::clamp(Dot(d, axis), -extent, extent);
            closest = closest + axis * distance;
        }
        return closest;
    }

    Vec3 ClosestPointOnSegment(const Vec3& point, const Vec3& a, const Vec3& b)
    {
        const Vec3 ab = b - a;
        const float lengthSquared = ab.LengthSquared();
        if (lengthSquared < kEpsilon)
        {
            return a;
        }
        const float t = std::clamp(Dot(point - a, ab) / lengthSquared, 0.0f, 1.0f);
        return a + ab * t;
    }

    // 두 선분(p1,q1), (p2,q2) 사이의 최단거리 제곱을 구하고, 각 선분 위의 최근접점을 c1/c2에 담는다.
    // Ericson, *Real-Time Collision Detection*, "Closest Point of Two Line Segments"의 표준 알고리즘.
    float ClosestSegmentSegmentDistanceSquared(const Vec3& p1, const Vec3& q1, const Vec3& p2, const Vec3& q2,
                                                Vec3& outC1, Vec3& outC2)
    {
        const Vec3 d1 = q1 - p1;
        const Vec3 d2 = q2 - p2;
        const Vec3 r = p1 - p2;
        const float a = Dot(d1, d1);
        const float e = Dot(d2, d2);
        const float f = Dot(d2, r);

        float s, t;
        if (a < kEpsilon && e < kEpsilon)
        {
            outC1 = p1;
            outC2 = p2;
            return (outC1 - outC2).LengthSquared();
        }
        if (a < kEpsilon)
        {
            s = 0.0f;
            t = std::clamp(f / e, 0.0f, 1.0f);
        }
        else
        {
            const float c = Dot(d1, r);
            if (e < kEpsilon)
            {
                t = 0.0f;
                s = std::clamp(-c / a, 0.0f, 1.0f);
            }
            else
            {
                const float b = Dot(d1, d2);
                const float denom = a * e - b * b;
                s = (std::abs(denom) > kEpsilon) ? std::clamp((b * f - c * e) / denom, 0.0f, 1.0f) : 0.0f;
                t = (b * s + f) / e;

                if (t < 0.0f)
                {
                    t = 0.0f;
                    s = std::clamp(-c / a, 0.0f, 1.0f);
                }
                else if (t > 1.0f)
                {
                    t = 1.0f;
                    s = std::clamp((b - c) / a, 0.0f, 1.0f);
                }
            }
        }
        outC1 = p1 + d1 * s;
        outC2 = p2 + d2 * t;
        return (outC1 - outC2).LengthSquared();
    }

    // OBB-OBB SAT(분리축 정리). Ericson, *Real-Time Collision Detection*, 4.4.1절의 표준 알고리즘.
    bool TestOBBOBB(const OBB& a, const OBB& b)
    {
        const Vec3 aAxes[3] = {a.GetAxis(0), a.GetAxis(1), a.GetAxis(2)};
        const Vec3 bAxes[3] = {b.GetAxis(0), b.GetAxis(1), b.GetAxis(2)};
        const float ea[3] = {a.halfExtents.x, a.halfExtents.y, a.halfExtents.z};
        const float eb[3] = {b.halfExtents.x, b.halfExtents.y, b.halfExtents.z};

        float r[3][3];
        float absR[3][3];
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                r[i][j] = Dot(aAxes[i], bAxes[j]);
                absR[i][j] = std::abs(r[i][j]) + kEpsilon;
            }
        }

        const Vec3 worldOffset = b.transform.position - a.transform.position;
        float t[3];
        for (int i = 0; i < 3; ++i)
        {
            t[i] = Dot(worldOffset, aAxes[i]);
        }

        // L = A0, A1, A2
        for (int i = 0; i < 3; ++i)
        {
            const float ra = ea[i];
            const float rb = eb[0] * absR[i][0] + eb[1] * absR[i][1] + eb[2] * absR[i][2];
            if (std::abs(t[i]) > ra + rb)
            {
                return false;
            }
        }

        // L = B0, B1, B2
        for (int j = 0; j < 3; ++j)
        {
            const float ra = ea[0] * absR[0][j] + ea[1] * absR[1][j] + ea[2] * absR[2][j];
            const float rb = eb[j];
            const float tProjected = t[0] * r[0][j] + t[1] * r[1][j] + t[2] * r[2][j];
            if (std::abs(tProjected) > ra + rb)
            {
                return false;
            }
        }

        // L = Ai x Bj (9축)
        float ra, rb;

        ra = ea[1] * absR[2][0] + ea[2] * absR[1][0];
        rb = eb[1] * absR[0][2] + eb[2] * absR[0][1];
        if (std::abs(t[2] * r[1][0] - t[1] * r[2][0]) > ra + rb)
            return false;

        ra = ea[1] * absR[2][1] + ea[2] * absR[1][1];
        rb = eb[0] * absR[0][2] + eb[2] * absR[0][0];
        if (std::abs(t[2] * r[1][1] - t[1] * r[2][1]) > ra + rb)
            return false;

        ra = ea[1] * absR[2][2] + ea[2] * absR[1][2];
        rb = eb[0] * absR[0][1] + eb[1] * absR[0][0];
        if (std::abs(t[2] * r[1][2] - t[1] * r[2][2]) > ra + rb)
            return false;

        ra = ea[0] * absR[2][0] + ea[2] * absR[0][0];
        rb = eb[1] * absR[1][2] + eb[2] * absR[1][1];
        if (std::abs(t[0] * r[2][0] - t[2] * r[0][0]) > ra + rb)
            return false;

        ra = ea[0] * absR[2][1] + ea[2] * absR[0][1];
        rb = eb[0] * absR[1][2] + eb[2] * absR[1][0];
        if (std::abs(t[0] * r[2][1] - t[2] * r[0][1]) > ra + rb)
            return false;

        ra = ea[0] * absR[2][2] + ea[2] * absR[0][2];
        rb = eb[0] * absR[1][1] + eb[1] * absR[1][0];
        if (std::abs(t[0] * r[2][2] - t[2] * r[0][2]) > ra + rb)
            return false;

        ra = ea[0] * absR[1][0] + ea[1] * absR[0][0];
        rb = eb[1] * absR[2][2] + eb[2] * absR[2][1];
        if (std::abs(t[1] * r[0][0] - t[0] * r[1][0]) > ra + rb)
            return false;

        ra = ea[0] * absR[1][1] + ea[1] * absR[0][1];
        rb = eb[0] * absR[2][2] + eb[2] * absR[2][0];
        if (std::abs(t[1] * r[0][1] - t[0] * r[1][1]) > ra + rb)
            return false;

        ra = ea[0] * absR[1][2] + ea[1] * absR[0][2];
        rb = eb[0] * absR[2][1] + eb[1] * absR[2][0];
        if (std::abs(t[1] * r[0][2] - t[0] * r[1][2]) > ra + rb)
            return false;

        return true;
    }

    OBB AsOBB(const AABB& box)
    {
        OBB obb(Transform(), box.Extents());
        obb.transform.position = box.Center();
        return obb;
    }
}

bool Contains(const AABB& box, const Vec3& point)
{
    return point.x >= box.min.x - kEpsilon && point.x <= box.max.x + kEpsilon && point.y >= box.min.y - kEpsilon &&
           point.y <= box.max.y + kEpsilon && point.z >= box.min.z - kEpsilon && point.z <= box.max.z + kEpsilon;
}

bool Contains(const OBB& box, const Vec3& point)
{
    const Vec3 d = point - box.transform.position;
    for (int i = 0; i < 3; ++i)
    {
        const float distance = Dot(d, box.GetAxis(i));
        if (std::abs(distance) > box.halfExtents[i] + kEpsilon)
        {
            return false;
        }
    }
    return true;
}

bool Intersects(const AABB& a, const AABB& b)
{
    return a.min.x <= b.max.x + kEpsilon && a.max.x >= b.min.x - kEpsilon && a.min.y <= b.max.y + kEpsilon &&
           a.max.y >= b.min.y - kEpsilon && a.min.z <= b.max.z + kEpsilon && a.max.z >= b.min.z - kEpsilon;
}

std::optional<AABB> IntersectionRegion(const AABB& a, const AABB& b)
{
    if (!Intersects(a, b))
    {
        return std::nullopt;
    }
    const Vec3 newMin(std::max(a.min.x, b.min.x), std::max(a.min.y, b.min.y), std::max(a.min.z, b.min.z));
    const Vec3 newMax(std::min(a.max.x, b.max.x), std::min(a.max.y, b.max.y), std::min(a.max.z, b.max.z));
    return AABB(newMin, newMax);
}

bool Intersects(const Sphere& a, const Sphere& b)
{
    return (b.center - a.center).Length() <= a.radius + b.radius + kEpsilon;
}

bool Intersects(const Sphere& sphere, const AABB& box)
{
    const Vec3 closest = ClosestPointOnAABB(sphere.center, box);
    return (closest - sphere.center).Length() <= sphere.radius + kEpsilon;
}

bool Intersects(const AABB& box, const Sphere& sphere)
{
    return Intersects(sphere, box);
}

bool Intersects(const Sphere& sphere, const OBB& box)
{
    const Vec3 closest = ClosestPointOnOBB(sphere.center, box);
    return (closest - sphere.center).Length() <= sphere.radius + kEpsilon;
}

bool Intersects(const OBB& box, const Sphere& sphere)
{
    return Intersects(sphere, box);
}

bool Intersects(const AABB& a, const OBB& b)
{
    return TestOBBOBB(AsOBB(a), b);
}

bool Intersects(const OBB& a, const AABB& b)
{
    return TestOBBOBB(a, AsOBB(b));
}

bool Intersects(const OBB& a, const OBB& b)
{
    return TestOBBOBB(a, b);
}

bool Intersects(const Sphere& sphere, const Capsule& capsule)
{
    const Vec3 closest = ClosestPointOnSegment(sphere.center, capsule.pointA, capsule.pointB);
    return (closest - sphere.center).Length() <= sphere.radius + capsule.radius + kEpsilon;
}

bool Intersects(const Capsule& capsule, const Sphere& sphere)
{
    return Intersects(sphere, capsule);
}

bool Intersects(const Capsule& a, const Capsule& b)
{
    Vec3 c1, c2;
    const float distanceSquared = ClosestSegmentSegmentDistanceSquared(a.pointA, a.pointB, b.pointA, b.pointB, c1, c2);
    const float radiusSum = a.radius + b.radius + kEpsilon;
    return distanceSquared <= radiusSum * radiusSum;
}

namespace
{
    // 선분과 볼록 도형(AABB/OBB) 사이의 최근접점을 번갈아 투영(alternating projection)해서 근사한다 —
    // 둘 다 볼록 집합이라 몇 번만 반복해도 빠르게 수렴한다. Tier 2 항목의 "닫힌 형태 해 대신 단순화 허용"
    // 방침에 따른 구현(전략 문서 참고).
    template <typename ClosestPointOnShapeFn>
    float ClosestSegmentShapeDistance(const Vec3& a, const Vec3& b, ClosestPointOnShapeFn closestPointOnShape,
                                       Vec3& outOnSegment, Vec3& outOnShape)
    {
        // 두 볼록 집합(선분, 박스) 사이의 번갈아 투영(alternating projection)은 기하급수적으로 수렴한다 —
        // 매 반복마다 오차가 일정 비율 이하로 줄어들어, 소수의 반복만으로도 부동소수점 정밀도 한계에
        // 도달한다. 20회는 이 비용이 무시할 만한 수준(핫패스 아님)임을 감안해 넉넉하게 잡은 값이며,
        // tests/IntersectionsTest.cpp의 브루트포스 샘플링 오라클 대조 테스트로 실제 수렴 정밀도를
        // 검증했다(대각선 방향 세그먼트 기준 1e-3 이내 일치).
        constexpr int kIterations = 20;
        Vec3 pointOnShape = closestPointOnShape(a);
        Vec3 pointOnSegment = a;
        for (int i = 0; i < kIterations; ++i)
        {
            pointOnSegment = ClosestPointOnSegment(pointOnShape, a, b);
            pointOnShape = closestPointOnShape(pointOnSegment);
        }
        outOnSegment = pointOnSegment;
        outOnShape = pointOnShape;
        return (pointOnSegment - pointOnShape).Length();
    }
}

bool Intersects(const Capsule& capsule, const AABB& box)
{
    Vec3 onSegment, onShape;
    const float distance = ClosestSegmentShapeDistance(
        capsule.pointA, capsule.pointB, [&box](const Vec3& p) { return ClosestPointOnAABB(p, box); }, onSegment,
        onShape);
    return distance <= capsule.radius + kEpsilon;
}

bool Intersects(const AABB& box, const Capsule& capsule)
{
    return Intersects(capsule, box);
}

bool Intersects(const Capsule& capsule, const OBB& box)
{
    Vec3 onSegment, onShape;
    const float distance = ClosestSegmentShapeDistance(
        capsule.pointA, capsule.pointB, [&box](const Vec3& p) { return ClosestPointOnOBB(p, box); }, onSegment,
        onShape);
    return distance <= capsule.radius + kEpsilon;
}

bool Intersects(const OBB& box, const Capsule& capsule)
{
    return Intersects(capsule, box);
}

bool Intersects(const Sphere& sphere, const Cylinder& cylinder)
{
    const Vec3 axis = cylinder.pointB - cylinder.pointA;
    const float axisLengthSquared = axis.LengthSquared();
    if (axisLengthSquared < kEpsilon)
    {
        // 퇴화한(길이 0) 실린더 — 구처럼 취급.
        return (sphere.center - cylinder.pointA).Length() <= sphere.radius + cylinder.radius + kEpsilon;
    }

    const float t = Dot(sphere.center - cylinder.pointA, axis) / axisLengthSquared;
    const float tClamped = std::clamp(t, 0.0f, 1.0f);
    const Vec3 axisPoint = cylinder.pointA + axis * tClamped;

    if (t >= 0.0f && t <= 1.0f)
    {
        // 옆면(측면) 케이스 — 축에 투영한 점까지의 수직 거리로 판단.
        return (sphere.center - axisPoint).Length() <= cylinder.radius + sphere.radius + kEpsilon;
    }

    // 캡(평평한 원판) 케이스 — axisPoint가 캡 평면 위에 있으므로, sphere.center를 축 방향/반경 방향으로 분해한다.
    const Vec3 axisDir = axis.Normalized();
    const Vec3 toCenter = sphere.center - axisPoint;
    const float alongAxisDistance = Dot(toCenter, axisDir);
    const Vec3 radialVector = toCenter - axisDir * alongAxisDistance;
    const float radialDistance = radialVector.Length();

    const Vec3 closestPoint =
        (radialDistance <= cylinder.radius) ? (axisPoint + radialVector)
                                             : (axisPoint + radialVector.Normalized() * cylinder.radius);
    return (sphere.center - closestPoint).Length() <= sphere.radius + kEpsilon;
}

bool Intersects(const Cylinder& cylinder, const Sphere& sphere)
{
    return Intersects(sphere, cylinder);
}
