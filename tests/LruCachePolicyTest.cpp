#include <gtest/gtest.h>

#include "platform/model_import/LruCachePolicy.h"

TEST(LruCachePolicyTest, KeepsEntryAliveWithoutExternalReference)
{
    LruCachePolicy policy(2);

    {
        auto model = std::make_shared<const Model>();
        policy.Store("key", model);
    }
    // RefCountingCachePolicy와 달리, LRU는 외부 참조가 없어도 최근 사용된 항목을 강하게 붙잡는다.
    EXPECT_NE(policy.TryGet("key"), nullptr);
}

TEST(LruCachePolicyTest, EvictsLeastRecentlyUsedEntryBeyondCapacity)
{
    LruCachePolicy policy(2);

    policy.Store("a", std::make_shared<const Model>());
    policy.Store("b", std::make_shared<const Model>());
    policy.Store("c", std::make_shared<const Model>());  // 용량(2) 초과 -> 가장 오래된 "a"가 축출됨

    EXPECT_EQ(policy.TryGet("a"), nullptr);
    EXPECT_NE(policy.TryGet("b"), nullptr);
    EXPECT_NE(policy.TryGet("c"), nullptr);
}

TEST(LruCachePolicyTest, TryGetRefreshesRecencyOrder)
{
    LruCachePolicy policy(2);

    policy.Store("a", std::make_shared<const Model>());
    policy.Store("b", std::make_shared<const Model>());
    policy.TryGet("a");  // "a"를 최근 사용으로 갱신 -> 다음 초과 시 "b"가 축출되어야 함

    policy.Store("c", std::make_shared<const Model>());

    EXPECT_NE(policy.TryGet("a"), nullptr);
    EXPECT_EQ(policy.TryGet("b"), nullptr);
    EXPECT_NE(policy.TryGet("c"), nullptr);
}

TEST(LruCachePolicyTest, UnloadForcesEvictionEvenWithinCapacity)
{
    LruCachePolicy policy(4);

    policy.Store("key", std::make_shared<const Model>());
    policy.Unload("key");

    EXPECT_EQ(policy.TryGet("key"), nullptr);
}
