#include <gtest/gtest.h>

#include "platform/geometry/AABB.h"

TEST(AABBTest, CenterIsMidpointOfMinAndMax)
{
    const AABB box(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 4.0f, 6.0f));
    EXPECT_TRUE(NearlyEqual(box.Center(), Vec3(1.0f, 2.0f, 3.0f)));
}

TEST(AABBTest, ExtentsIsHalfOfTheFullSize)
{
    const AABB box(Vec3(-1.0f, -2.0f, -3.0f), Vec3(1.0f, 2.0f, 3.0f));
    EXPECT_TRUE(NearlyEqual(box.Extents(), Vec3(1.0f, 2.0f, 3.0f)));
}
