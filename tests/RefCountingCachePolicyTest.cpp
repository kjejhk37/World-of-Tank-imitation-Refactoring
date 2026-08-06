#include <gtest/gtest.h>

#include "platform/model_import/RefCountingCachePolicy.h"

TEST(RefCountingCachePolicyTest, TryGetReturnsStoredModelWhileAlive)
{
    RefCountingCachePolicy policy;
    auto model = std::make_shared<const Model>();

    policy.Store("key", model);

    EXPECT_EQ(policy.TryGet("key"), model);
}

TEST(RefCountingCachePolicyTest, TryGetReturnsNullptrForUnknownKey)
{
    RefCountingCachePolicy policy;
    EXPECT_EQ(policy.TryGet("missing"), nullptr);
}

TEST(RefCountingCachePolicyTest, EntryIsAutomaticallyEvictedWhenLastReferenceIsReleased)
{
    RefCountingCachePolicy policy;
    {
        auto model = std::make_shared<const Model>();
        policy.Store("key", model);
        ASSERT_NE(policy.TryGet("key"), nullptr);
    }
    // model의 마지막 shared_ptr 소유자가 스코프를 벗어나 소멸했으므로, 캐시는 더 이상 살려주지 않는다.
    EXPECT_EQ(policy.TryGet("key"), nullptr);
}

TEST(RefCountingCachePolicyTest, UnloadForcesEvictionEvenWhileExternallyReferenced)
{
    RefCountingCachePolicy policy;
    auto model = std::make_shared<const Model>();
    policy.Store("key", model);

    policy.Unload("key");

    // 호출자(model)가 여전히 참조를 들고 있어도, 캐시 조회는 더 이상 이 엔트리를 돌려주지 않는다.
    EXPECT_EQ(policy.TryGet("key"), nullptr);
}
