#include <gtest/gtest.h>

#include "platform/model_import/ModelLoader.h"

namespace
{
std::string FixturePath(const std::string& fileName)
{
    return std::string(MODEL_IMPORT_FIXTURES_DIR) + fileName;
}
}  // namespace

TEST(ModelLoaderTest, LoadOBJProducesSingleNodeModelWithTriangulatedCube)
{
    const Model model = ModelLoader::LoadOBJ(FixturePath("cube.obj"));

    ASSERT_EQ(model.meshes.size(), 1u);
    const ModelMesh& mesh = *model.meshes[0];

    EXPECT_EQ(mesh.positions.size(), 36u);
    EXPECT_EQ(mesh.normals.size(), 36u);
    ASSERT_EQ(mesh.uvChannels.size(), 1u);
    EXPECT_EQ(mesh.uvChannels[0].size(), 36u);
    EXPECT_EQ(mesh.indices.size(), 36u);
    EXPECT_EQ(mesh.indices.size() % 3, 0u);

    ASSERT_TRUE(model.rootNode.meshIndex.has_value());
    EXPECT_EQ(*model.rootNode.meshIndex, 0u);
    EXPECT_TRUE(model.rootNode.children.empty());
}

TEST(ModelLoaderTest, LoadFBXProducesAtLeastOneTriangulatedMesh)
{
    const Model model = ModelLoader::LoadFBX(FixturePath("cube.fbx"));

    ASSERT_FALSE(model.meshes.empty());

    bool foundNonEmptyMesh = false;
    for (const std::shared_ptr<const ModelMesh>& mesh : model.meshes)
    {
        if (!mesh->positions.empty())
        {
            foundNonEmptyMesh = true;
            EXPECT_EQ(mesh->indices.size() % 3, 0u);
            EXPECT_EQ(mesh->positions.size(), mesh->normals.size());
        }
    }
    EXPECT_TRUE(foundNonEmptyMesh);
}

TEST(ModelLoaderTest, LoadGLTFProducesSingleTriangleMesh)
{
    const Model model = ModelLoader::LoadGLTF(FixturePath("triangle.gltf"));

    ASSERT_EQ(model.meshes.size(), 1u);
    const ModelMesh& mesh = *model.meshes[0];
    EXPECT_EQ(mesh.positions.size(), 3u);
    EXPECT_EQ(mesh.indices.size(), 3u);

    ASSERT_EQ(model.rootNode.children.size(), 1u);
    ASSERT_TRUE(model.rootNode.children[0].meshIndex.has_value());
    EXPECT_EQ(*model.rootNode.children[0].meshIndex, 0u);
}

TEST(ModelLoaderTest, LoadGLBSharesImplementationWithLoadGLTF)
{
    // LoadGLB/LoadGLTF는 둘 다 CgltfWrapper(cgltf_parse_file은 GLB/GLTF를 컨텐츠로 자동 판별)를
    // 그대로 호출하므로, 같은 .gltf 파일로도 LoadGLB가 동일하게 동작하는지 확인한다.
    const Model model = ModelLoader::LoadGLB(FixturePath("triangle.gltf"));

    ASSERT_EQ(model.meshes.size(), 1u);
    EXPECT_EQ(model.meshes[0]->positions.size(), 3u);
}

TEST(ModelLoaderTest, LoadGLTFPreservesSceneGraphInstancingWithoutFlattening)
{
    const Model model = ModelLoader::LoadGLTF(FixturePath("instancing.gltf"));

    // 같은 메쉬를 두 노드가 참조하므로 고유 지오메트리는 하나만 저장되어야 한다.
    ASSERT_EQ(model.meshes.size(), 1u);

    ASSERT_EQ(model.rootNode.children.size(), 2u);
    const Node& instanceA = model.rootNode.children[0];
    const Node& instanceB = model.rootNode.children[1];

    ASSERT_TRUE(instanceA.meshIndex.has_value());
    ASSERT_TRUE(instanceB.meshIndex.has_value());
    EXPECT_EQ(*instanceA.meshIndex, *instanceB.meshIndex);

    EXPECT_NE(instanceA.localTransform.position.x, instanceB.localTransform.position.x);
}
