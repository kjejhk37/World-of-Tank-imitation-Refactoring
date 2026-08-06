#include <gtest/gtest.h>

#include "platform/model_import/MeshManager.h"
#include "platform/model_import/NaiveSignatureDeduplicator.h"
#include "platform/model_import/RefCountingCachePolicy.h"

namespace
{
std::shared_ptr<ModelMesh> MakeTriangleMesh()
{
    auto mesh = std::make_shared<ModelMesh>();
    mesh->positions = {Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f)};
    mesh->indices = {0, 1, 2};
    mesh->normals = {Vec3::Zero(), Vec3::Zero(), Vec3::Zero()};
    mesh->uvChannels.resize(1);
    mesh->uvChannels[0] = {Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f)};
    return mesh;
}
}  // namespace

TEST(MeshManagerTest, GetOrLoadCallsLoaderOnlyOnceForSameKey)
{
    MeshManager manager(std::make_unique<RefCountingCachePolicy>());
    int loadCount = 0;
    auto loader = [&loadCount]() {
        ++loadCount;
        return Model();
    };

    const std::shared_ptr<const Model> first = manager.GetOrLoad("key", loader);
    const std::shared_ptr<const Model> second = manager.GetOrLoad("key", loader);

    EXPECT_EQ(loadCount, 1);
    EXPECT_EQ(first, second);
}

TEST(MeshManagerTest, UnloadForcesNextGetOrLoadToCallLoaderAgain)
{
    MeshManager manager(std::make_unique<RefCountingCachePolicy>());
    int loadCount = 0;
    auto loader = [&loadCount]() {
        ++loadCount;
        return Model();
    };

    const std::shared_ptr<const Model> first = manager.GetOrLoad("key", loader);
    manager.Unload("key");
    const std::shared_ptr<const Model> second = manager.GetOrLoad("key", loader);

    EXPECT_EQ(loadCount, 2);
}

TEST(MeshManagerTest, DeduplicatorReplacesCongruentMeshAcrossDifferentKeys)
{
    MeshManager manager(std::make_unique<RefCountingCachePolicy>(), std::make_unique<NaiveSignatureDeduplicator>());

    auto loader = []() {
        Model model;
        model.meshes.push_back(MakeTriangleMesh());
        model.rootNode.meshIndex = 0;
        return model;
    };

    const std::shared_ptr<const Model> modelA = manager.GetOrLoad("fileA", loader);
    const std::shared_ptr<const Model> modelB = manager.GetOrLoad("fileB", loader);

    ASSERT_EQ(modelA->meshes.size(), 1u);
    ASSERT_EQ(modelB->meshes.size(), 1u);
    // 서로 다른 키(파일)에서 왔지만 지오메트리가 동일하므로, 같은 ModelMesh 핸들로 교체되어야 한다.
    EXPECT_EQ(modelA->meshes[0], modelB->meshes[0]);
    // Model 자체는 캐시 단위(키별)로 별개 인스턴스다.
    EXPECT_NE(modelA, modelB);
}

TEST(MeshManagerTest, WithoutDeduplicatorDifferentKeysProduceIndependentMeshes)
{
    MeshManager manager(std::make_unique<RefCountingCachePolicy>());

    auto loader = []() {
        Model model;
        model.meshes.push_back(MakeTriangleMesh());
        model.rootNode.meshIndex = 0;
        return model;
    };

    const std::shared_ptr<const Model> modelA = manager.GetOrLoad("fileA", loader);
    const std::shared_ptr<const Model> modelB = manager.GetOrLoad("fileB", loader);

    EXPECT_NE(modelA->meshes[0], modelB->meshes[0]);
}
