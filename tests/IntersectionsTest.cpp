#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>

#include "platform/geometry/Intersections.h"

// ---- Contains(AABB, point) ----

TEST(IntersectionsTest, AABBContainsPointInside)
{
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Contains(box, Vec3(0.0f, 0.0f, 0.0f)));
}

TEST(IntersectionsTest, AABBDoesNotContainPointOutside)
{
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Contains(box, Vec3(5.0f, 0.0f, 0.0f)));
}

TEST(IntersectionsTest, AABBContainsPointOnBoundary)
{
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Contains(box, Vec3(1.0f, 0.0f, 0.0f)));
}

// ---- Contains(OBB, point) ----

TEST(IntersectionsTest, OBBContainsPointInsideAxisAligned)
{
    const OBB box(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Contains(box, Vec3(0.5f, 0.5f, 0.5f)));
}

TEST(IntersectionsTest, OBBDoesNotContainPointOutsideAxisAligned)
{
    const OBB box(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Contains(box, Vec3(5.0f, 0.0f, 0.0f)));
}

TEST(IntersectionsTest, OBBContainsPointInsideAfterRotation)
{
    Transform transform;
    transform.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 45.0f);
    const OBB box(transform, Vec3(1.0f, 1.0f, 1.0f));

    // 회전 후에도 중심점은 항상 내부에 있어야 한다.
    EXPECT_TRUE(Contains(box, transform.position));
    // 회전 전 로컬 좌표계 기준으로 바깥쪽이었던 점(월드 축 기준 (2,0,0))은 45도 회전 후에도 halfExtent(1)를 벗어난다.
    EXPECT_FALSE(Contains(box, Vec3(2.0f, 0.0f, 0.0f)));
}

// ---- Intersects(AABB, AABB) / IntersectionRegion ----

TEST(IntersectionsTest, OverlappingAABBsIntersect)
{
    const AABB a(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 2.0f));
    const AABB b(Vec3(1.0f, 1.0f, 1.0f), Vec3(3.0f, 3.0f, 3.0f));
    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, SeparatedAABBsDoNotIntersect)
{
    const AABB a(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const AABB b(Vec3(10.0f, 10.0f, 10.0f), Vec3(11.0f, 11.0f, 11.0f));
    EXPECT_FALSE(Intersects(a, b));
}

TEST(IntersectionsTest, IntersectionRegionOfOverlappingAABBsIsTheOverlapBox)
{
    const AABB a(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 2.0f));
    const AABB b(Vec3(1.0f, 1.0f, 1.0f), Vec3(3.0f, 3.0f, 3.0f));

    const std::optional<AABB> region = IntersectionRegion(a, b);
    ASSERT_TRUE(region.has_value());
    EXPECT_TRUE(NearlyEqual(region->min, Vec3(1.0f, 1.0f, 1.0f)));
    EXPECT_TRUE(NearlyEqual(region->max, Vec3(2.0f, 2.0f, 2.0f)));
}

TEST(IntersectionsTest, IntersectionRegionOfSeparatedAABBsIsNullopt)
{
    const AABB a(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
    const AABB b(Vec3(10.0f, 10.0f, 10.0f), Vec3(11.0f, 11.0f, 11.0f));
    EXPECT_FALSE(IntersectionRegion(a, b).has_value());
}

// ---- Intersects(Sphere, Sphere) ----

TEST(IntersectionsTest, OverlappingSpheresIntersect)
{
    EXPECT_TRUE(Intersects(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f), Sphere(Vec3(1.5f, 0.0f, 0.0f), 1.0f)));
}

TEST(IntersectionsTest, SeparatedSpheresDoNotIntersect)
{
    EXPECT_FALSE(Intersects(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f), Sphere(Vec3(10.0f, 0.0f, 0.0f), 1.0f)));
}

TEST(IntersectionsTest, TouchingSpheresIntersect)
{
    EXPECT_TRUE(Intersects(Sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f), Sphere(Vec3(2.0f, 0.0f, 0.0f), 1.0f)));
}

// ---- Intersects(Sphere, AABB) ----

TEST(IntersectionsTest, SphereOverlappingAABBIntersect)
{
    const Sphere sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    const AABB box(Vec3(0.5f, 0.5f, 0.5f), Vec3(2.0f, 2.0f, 2.0f));
    EXPECT_TRUE(Intersects(sphere, box));
    EXPECT_TRUE(Intersects(box, sphere));
}

TEST(IntersectionsTest, SphereFarFromAABBDoesNotIntersect)
{
    const Sphere sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
    const AABB box(Vec3(10.0f, 10.0f, 10.0f), Vec3(11.0f, 11.0f, 11.0f));
    EXPECT_FALSE(Intersects(sphere, box));
    EXPECT_FALSE(Intersects(box, sphere));
}

// ---- Intersects(Sphere, OBB) ----

TEST(IntersectionsTest, SphereOverlappingRotatedOBBIntersect)
{
    Transform transform;
    transform.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 45.0f);
    const OBB box(transform, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_TRUE(Intersects(Sphere(transform.position, 0.5f), box));
    EXPECT_TRUE(Intersects(box, Sphere(transform.position, 0.5f)));
}

TEST(IntersectionsTest, SphereFarFromOBBDoesNotIntersect)
{
    const OBB box(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Intersects(Sphere(Vec3(10.0f, 10.0f, 10.0f), 1.0f), box));
}

// ---- Intersects(AABB, OBB) ----

TEST(IntersectionsTest, OverlappingAABBAndAxisAlignedOBBIntersect)
{
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    const OBB obb(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Intersects(box, obb));
    EXPECT_TRUE(Intersects(obb, box));
}

TEST(IntersectionsTest, SeparatedAABBAndOBBDoNotIntersect)
{
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    Transform transform;
    transform.position = Vec3(20.0f, 20.0f, 20.0f);
    const OBB obb(transform, Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Intersects(box, obb));
    EXPECT_FALSE(Intersects(obb, box));
}

// ---- Intersects(OBB, OBB) ----

TEST(IntersectionsTest, OverlappingAxisAlignedOBBsIntersect)
{
    const OBB a(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    Transform transformB;
    transformB.position = Vec3(1.5f, 0.0f, 0.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, SeparatedOBBsDoNotIntersect)
{
    const OBB a(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    Transform transformB;
    transformB.position = Vec3(20.0f, 0.0f, 0.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Intersects(a, b));
}

TEST(IntersectionsTest, RotatedOBBCornerPokingIntoAxisAlignedOBBIntersects)
{
    const OBB a(Transform(), Vec3(1.0f, 1.0f, 1.0f));

    Transform transformB;
    // 회전된 정육면체의 대각선 반경(sqrt(2))만큼 떨어뜨려, 모서리가 a쪽으로 살짝 파고들게 배치.
    transformB.position = Vec3(1.0f + std::sqrt(2.0f) - 0.1f, 0.0f, 0.0f);
    transformB.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), 45.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, RotatedOBBFarAwayDoesNotIntersect)
{
    const OBB a(Transform(), Vec3(1.0f, 1.0f, 1.0f));

    Transform transformB;
    transformB.position = Vec3(10.0f, 0.0f, 0.0f);
    transformB.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), 45.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_FALSE(Intersects(a, b));
}

// SAT은 15개 후보 분리축(각자 3개 + 축끼리 외적 9개)을 검사한다. 위 테스트들은 전부 같은 Z축 회전이라
// 특정 교차축 블록의 복붙 실수를 놓칠 수 있다 — 서로 다른 축(X, Y) 회전으로 다양성을 넓힌다.
TEST(IntersectionsTest, OBBsRotatedOnXAxisOverlapWhenCloseEnough)
{
    const OBB a(Transform(), Vec3(1.0f, 1.0f, 1.0f));

    Transform transformB;
    transformB.position = Vec3(0.0f, 1.0f + std::sqrt(2.0f) - 0.1f, 0.0f);
    transformB.rotation = Quaternion::FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), 45.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, OBBsRotatedOnYAxisSeparateWhenFarEnough)
{
    const OBB a(Transform(), Vec3(1.0f, 1.0f, 1.0f));

    Transform transformB;
    transformB.position = Vec3(0.0f, 0.0f, 10.0f);
    transformB.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 45.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_FALSE(Intersects(a, b));
}

TEST(IntersectionsTest, OBBsRotatedOnDifferentAxesAtSameCenterAlwaysOverlap)
{
    // 두 박스가 서로 다른 축(X, Y)으로 각각 회전해 있어도, 중심이 같으면 항상 겹쳐야 한다 —
    // 공유하는 중심점이 항상 두 박스 모두의 내부에 있기 때문이다(회전과 무관하게 자명하게 참).
    // 이 자명한 사실을 이용해, 서로 다른 회전 조합으로 실제 계산되는 SAT의 9개 교차축이
    // 거짓 분리(오탐)를 내지 않는지 손으로 좌표를 유도하지 않고도 신뢰성 있게 검증한다.
    Transform transformA;
    transformA.rotation = Quaternion::FromAxisAngle(Vec3(1.0f, 0.0f, 0.0f), 30.0f);
    const OBB a(transformA, Vec3(1.0f, 1.0f, 1.0f));

    Transform transformB;
    transformB.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 40.0f);
    const OBB b(transformB, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, OBBIntersectionIsSymmetricRegardlessOfArgumentOrder)
{
    const OBB axisAligned(Transform(), Vec3(1.0f, 1.0f, 1.0f));

    Transform rotatedClose;
    rotatedClose.position = Vec3(1.5f, 0.0f, 0.0f);
    rotatedClose.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 20.0f);
    const OBB overlapping(rotatedClose, Vec3(1.0f, 1.0f, 1.0f));

    Transform rotatedFar;
    rotatedFar.position = Vec3(0.0f, 20.0f, 0.0f);
    rotatedFar.rotation = Quaternion::FromAxisAngle(Vec3(1.0f, 1.0f, 0.0f), 35.0f);
    const OBB separated(rotatedFar, Vec3(1.0f, 1.0f, 1.0f));

    EXPECT_EQ(Intersects(axisAligned, overlapping), Intersects(overlapping, axisAligned));
    EXPECT_EQ(Intersects(axisAligned, separated), Intersects(separated, axisAligned));
}

// ---- Intersects(Sphere, Capsule) ----

TEST(IntersectionsTest, SphereOverlappingCapsuleIntersect)
{
    const Capsule capsule(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Sphere sphere(Vec3(0.0f, 0.6f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(sphere, capsule));
    EXPECT_TRUE(Intersects(capsule, sphere));
}

TEST(IntersectionsTest, SphereFarFromCapsuleDoesNotIntersect)
{
    const Capsule capsule(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Sphere sphere(Vec3(0.0f, 10.0f, 0.0f), 0.5f);
    EXPECT_FALSE(Intersects(sphere, capsule));
    EXPECT_FALSE(Intersects(capsule, sphere));
}

TEST(IntersectionsTest, SphereNearCapsuleEndCapIntersects)
{
    const Capsule capsule(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    // 선분 바깥쪽(캡 부분)에서 겹치는지 확인 — 반구 캡 처리가 되는지 검증.
    const Sphere sphere(Vec3(2.6f, 0.0f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(sphere, capsule));
}

// ---- Intersects(Capsule, Capsule) ----

TEST(IntersectionsTest, CrossingCapsulesIntersect)
{
    const Capsule a(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Capsule b(Vec3(0.0f, -2.0f, 0.0f), Vec3(0.0f, 2.0f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, ParallelSeparatedCapsulesDoNotIntersect)
{
    const Capsule a(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Capsule b(Vec3(-2.0f, 5.0f, 0.0f), Vec3(2.0f, 5.0f, 0.0f), 0.5f);
    EXPECT_FALSE(Intersects(a, b));
}

TEST(IntersectionsTest, ParallelOverlappingCapsulesIntersect)
{
    const Capsule a(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Capsule b(Vec3(-2.0f, 0.9f, 0.0f), Vec3(2.0f, 0.9f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(a, b));
}

TEST(IntersectionsTest, SkewCapsulesFarApartDoNotIntersect)
{
    const Capsule a(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 10.0f), 0.5f);
    const Capsule b(Vec3(-2.0f, 20.0f, 0.0f), Vec3(2.0f, 20.0f, 10.0f), 0.5f);
    EXPECT_FALSE(Intersects(a, b));
}

// ---- Intersects(Capsule, AABB) — Tier 2 ----

TEST(IntersectionsTest, CapsulePiercingAABBIntersects)
{
    const Capsule capsule(Vec3(0.0f, 5.0f, 0.0f), Vec3(0.0f, -5.0f, 0.0f), 0.5f);
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Intersects(capsule, box));
    EXPECT_TRUE(Intersects(box, capsule));
}

TEST(IntersectionsTest, CapsuleFarFromAABBDoesNotIntersect)
{
    const Capsule capsule(Vec3(20.0f, 5.0f, 0.0f), Vec3(20.0f, -5.0f, 0.0f), 0.5f);
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(Intersects(capsule, box));
    EXPECT_FALSE(Intersects(box, capsule));
}

TEST(IntersectionsTest, CapsuleJustGrazingAABBCornerIntersects)
{
    // 캡슐이 상자 바깥을 지나가지만, 반지름이 모서리까지 닿을 만큼 커서 겹쳐야 한다.
    const Capsule capsule(Vec3(2.0f, 5.0f, 2.0f), Vec3(2.0f, -5.0f, 2.0f), 1.5f);
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Intersects(capsule, box));
}

TEST(IntersectionsTest, CapsuleAABBAlternatingProjectionMatchesBruteForceSamplingAtTightMargin)
{
    // Capsule-AABB는 닫힌 형태 해 대신 alternating-projection(번갈아 투영) 반복으로 근사한다
    // (전략/리뷰 문서 참고). 대각선 방향(축에 정렬되지 않은) 세그먼트에 대해, 세그먼트를 촘촘히
    // 샘플링해 각 샘플-박스 최단거리의 최솟값을 구하는 브루트포스 오라클과 대조해 실제 수렴 정밀도를
    // 검증한다 — 손으로 정확한 최단거리를 유도하지 않고도 신뢰할 수 있는 기준값을 얻는다.
    const AABB box(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    const Vec3 segmentStart(3.0f, 3.0f, 0.5f);
    const Vec3 segmentEnd(0.5f, 0.3f, 3.0f);

    float bruteForceMinDistance = std::numeric_limits<float>::max();
    constexpr int kSamples = 2000;
    for (int i = 0; i <= kSamples; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(kSamples);
        const Vec3 p = segmentStart + (segmentEnd - segmentStart) * t;
        const Vec3 closest(std::clamp(p.x, box.min.x, box.max.x), std::clamp(p.y, box.min.y, box.max.y),
                            std::clamp(p.z, box.min.z, box.max.z));
        bruteForceMinDistance = std::min(bruteForceMinDistance, (p - closest).Length());
    }

    const Capsule justTouching(segmentStart, segmentEnd, bruteForceMinDistance + 1e-3f);
    const Capsule justSeparated(segmentStart, segmentEnd, bruteForceMinDistance - 1e-3f);

    EXPECT_TRUE(Intersects(justTouching, box));
    EXPECT_FALSE(Intersects(justSeparated, box));
}

// ---- Intersects(Capsule, OBB) — Tier 2 ----

TEST(IntersectionsTest, CapsulePiercingRotatedOBBIntersects)
{
    Transform transform;
    transform.rotation = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 45.0f);
    const OBB box(transform, Vec3(1.0f, 1.0f, 1.0f));

    const Capsule capsule(Vec3(0.0f, 5.0f, 0.0f), Vec3(0.0f, -5.0f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(capsule, box));
    EXPECT_TRUE(Intersects(box, capsule));
}

TEST(IntersectionsTest, CapsuleFarFromOBBDoesNotIntersect)
{
    const OBB box(Transform(), Vec3(1.0f, 1.0f, 1.0f));
    const Capsule capsule(Vec3(20.0f, 5.0f, 0.0f), Vec3(20.0f, -5.0f, 0.0f), 0.5f);
    EXPECT_FALSE(Intersects(capsule, box));
    EXPECT_FALSE(Intersects(box, capsule));
}

// ---- Intersects(Sphere, Cylinder) — Tier 2 ----

TEST(IntersectionsTest, SphereOverlappingCylinderSideIntersects)
{
    const Cylinder cylinder(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Sphere sphere(Vec3(0.0f, 0.6f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(sphere, cylinder));
    EXPECT_TRUE(Intersects(cylinder, sphere));
}

TEST(IntersectionsTest, SphereFarFromCylinderDoesNotIntersect)
{
    const Cylinder cylinder(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 0.5f);
    const Sphere sphere(Vec3(0.0f, 10.0f, 0.0f), 0.5f);
    EXPECT_FALSE(Intersects(sphere, cylinder));
}

TEST(IntersectionsTest, SphereInFrontOfFlatCapWithinRadiusIntersects)
{
    const Cylinder cylinder(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 1.0f);
    // 캡 평면(x=2) 바로 앞, 반지름(1.0) 안쪽 위치 — 평평한 캡과의 수직 거리로 판정되어야 한다.
    const Sphere sphere(Vec3(2.4f, 0.3f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(sphere, cylinder));
}

TEST(IntersectionsTest, SphereBeyondCapButOutsideRadiusDoesNotIntersectViaCapPlaneShortcut)
{
    // 캡 평면 너머 + 반지름 바깥(테두리 바깥) — 테두리(rim)까지의 거리로 판정되어 접촉이 없어야 한다.
    const Cylinder cylinder(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 1.0f);
    const Sphere sphere(Vec3(2.3f, 3.0f, 0.0f), 0.3f);
    EXPECT_FALSE(Intersects(sphere, cylinder));
}

TEST(IntersectionsTest, SphereNearCapRimIntersects)
{
    const Cylinder cylinder(Vec3(-2.0f, 0.0f, 0.0f), Vec3(2.0f, 0.0f, 0.0f), 1.0f);
    // 캡 테두리(반지름 1.0, x=2 평면) 근처 — 테두리까지의 최단거리로 판정되어야 한다.
    const Sphere sphere(Vec3(2.3f, 1.1f, 0.0f), 0.5f);
    EXPECT_TRUE(Intersects(sphere, cylinder));
}
