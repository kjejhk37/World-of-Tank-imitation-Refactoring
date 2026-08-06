#include <gtest/gtest.h>

#include <cmath>

#include "platform/collision/conservative_advancement/ConservativeAdvancement.h"

TEST(ConservativeAdvancementTest, HeadOnApproachMatchesAnalyticTimeOfImpact)
{
    // 두 AABB(반지름 sqrt(3)의 경계 구) 하나는 정지, 하나는 정면으로 접근 — 해석적 TOI와 비교.
    const Geometry a = AABB(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));       // center (0,0,0)
    const Geometry b = AABB(Vec3(99.0f, -1.0f, -1.0f), Vec3(101.0f, 1.0f, 1.0f));     // center (100,0,0)
    const Vec3 velocityA(10.0f, 0.0f, 0.0f);
    const Vec3 velocityB(0.0f, 0.0f, 0.0f);

    const ToiResult result = ConservativeAdvancement(a, velocityA, b, velocityB, 10.0f);

    const float combinedRadius = 2.0f * std::sqrt(3.0f);
    const float expectedToi = (100.0f - combinedRadius) / 10.0f;

    EXPECT_TRUE(result.collided);
    EXPECT_NEAR(result.timeOfImpact, expectedToi, 1e-2f);
}

TEST(ConservativeAdvancementTest, SeparatingObjectsNeverCollide)
{
    const Geometry a = AABB(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    const Geometry b = AABB(Vec3(99.0f, -1.0f, -1.0f), Vec3(101.0f, 1.0f, 1.0f));
    const Vec3 velocityA(-10.0f, 0.0f, 0.0f);  // 멀어지는 방향
    const Vec3 velocityB(0.0f, 0.0f, 0.0f);

    const ToiResult result = ConservativeAdvancement(a, velocityA, b, velocityB, 10.0f);
    EXPECT_FALSE(result.collided);
}

TEST(ConservativeAdvancementTest, GrazingPathThatMissesDoesNotCollide)
{
    // A가 y축을 따라 지나가지만 B와의 최소 거리(5)가 경계 구 합(약 3.46)보다 커서 절대 닿지 않는다.
    const Geometry a = AABB(Vec3(-1.0f, -21.0f, -1.0f), Vec3(1.0f, -19.0f, 1.0f));  // center (0,-20,0)
    const Geometry b = AABB(Vec3(4.0f, -1.0f, -1.0f), Vec3(6.0f, 1.0f, 1.0f));      // center (5,0,0)
    const Vec3 velocityA(0.0f, 5.0f, 0.0f);
    const Vec3 velocityB(0.0f, 0.0f, 0.0f);

    const ToiResult result = ConservativeAdvancement(a, velocityA, b, velocityB, 10.0f);
    EXPECT_FALSE(result.collided);
}

TEST(ConservativeAdvancementTest, FastMovingObjectDoesNotTunnelThroughThinWall)
{
    // 한 프레임 시작/끝만 봤다면(discrete) x=0 -> x=1000으로 건너뛰어 x=50의 얇은 벽을 완전히 지나쳐버렸을
    // 상황 — CCD는 그 사이의 실제 충돌 시각을 찾아내야 한다.
    const Geometry projectile = AABB(Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f));  // center (0,0,0)
    const Geometry wall = AABB(Vec3(49.9f, -10.0f, -10.0f), Vec3(50.1f, 10.0f, 10.0f));   // 얇은 벽, center (50,0,0)
    const Vec3 velocityProjectile(1000.0f, 0.0f, 0.0f);
    const Vec3 velocityWall(0.0f, 0.0f, 0.0f);

    const ToiResult result = ConservativeAdvancement(projectile, velocityProjectile, wall, velocityWall, 1.0f);

    EXPECT_TRUE(result.collided);
    EXPECT_LT(result.timeOfImpact, 1.0f);
    EXPECT_GT(result.timeOfImpact, 0.0f);
}
