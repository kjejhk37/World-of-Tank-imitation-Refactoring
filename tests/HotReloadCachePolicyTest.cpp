#include <gtest/gtest.h>

#include "platform/model_import/HotReloadCachePolicy.h"

namespace
{
class StubFileChangeDetector : public IFileChangeDetector
{
public:
    bool HasChanged(const std::string& /*path*/) override
    {
        return changed;
    }

    bool changed = false;
};
}  // namespace

TEST(HotReloadCachePolicyTest, ReturnsStoredModelWhenNoChangeDetected)
{
    auto stub = std::make_unique<StubFileChangeDetector>();
    HotReloadCachePolicy policy(std::move(stub));

    auto model = std::make_shared<const Model>();
    policy.Store("key", model);

    EXPECT_EQ(policy.TryGet("key"), model);
}

TEST(HotReloadCachePolicyTest, TryGetReturnsNullptrAndClearsEntryWhenFileChanged)
{
    auto stub = std::make_unique<StubFileChangeDetector>();
    StubFileChangeDetector* stubPtr = stub.get();
    HotReloadCachePolicy policy(std::move(stub));

    policy.Store("key", std::make_shared<const Model>());
    stubPtr->changed = true;

    EXPECT_EQ(policy.TryGet("key"), nullptr);
}

TEST(HotReloadCachePolicyTest, UnloadRemovesEntryRegardlessOfChangeState)
{
    auto stub = std::make_unique<StubFileChangeDetector>();
    HotReloadCachePolicy policy(std::move(stub));

    policy.Store("key", std::make_shared<const Model>());
    policy.Unload("key");

    EXPECT_EQ(policy.TryGet("key"), nullptr);
}
