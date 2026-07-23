#include "model_import/UfbxWrapper.h"

#include <cstring>
#include <stdexcept>
#include <unordered_map>

#include "math/Quaternion.h"
#include "ufbx.h"

namespace
{

Vec3 ToVec3(const ufbx_vec3& v)
{
    return Vec3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}

Quaternion ToQuaternion(const ufbx_quat& q)
{
    return Quaternion(static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z),
                       static_cast<float>(q.w));
}

ModelMesh ConvertMesh(const ufbx_mesh* ufbxMesh)
{
    ModelMesh mesh;
    mesh.uvChannels.resize(1);

    std::vector<uint32_t> triIndices(ufbxMesh->max_face_triangles * 3);

    for (size_t faceIdx = 0; faceIdx < ufbxMesh->faces.count; ++faceIdx)
    {
        const ufbx_face face = ufbxMesh->faces.data[faceIdx];
        const uint32_t numTriangles =
            ufbx_triangulate_face(triIndices.data(), triIndices.size(), ufbxMesh, face);

        for (uint32_t t = 0; t < numTriangles; ++t)
        {
            for (int k = 0; k < 3; ++k)
            {
                const uint32_t vertexIndex = triIndices[static_cast<size_t>(t) * 3 + static_cast<size_t>(k)];

                mesh.positions.push_back(ToVec3(ufbx_get_vertex_vec3(&ufbxMesh->vertex_position, vertexIndex)));

                Vec3 normal = Vec3::Zero();
                if (ufbxMesh->vertex_normal.exists)
                {
                    normal = ToVec3(ufbx_get_vertex_vec3(&ufbxMesh->vertex_normal, vertexIndex));
                }
                mesh.normals.push_back(normal);

                Vec2 uv(0.0f, 0.0f);
                if (ufbxMesh->vertex_uv.exists)
                {
                    const ufbx_vec2 v = ufbx_get_vertex_vec2(&ufbxMesh->vertex_uv, vertexIndex);
                    uv = Vec2(static_cast<float>(v.x), static_cast<float>(v.y));
                }
                mesh.uvChannels[0].push_back(uv);

                mesh.indices.push_back(static_cast<uint32_t>(mesh.indices.size()));
            }
        }
    }

    if (ufbxMesh->materials.count > 0 && ufbxMesh->materials.data[0] != nullptr)
    {
        const ufbx_string& name = ufbxMesh->materials.data[0]->name;
        mesh.materialRef = std::string(name.data, name.length);
    }

    return mesh;
}

// meshRegistry는 ufbx_mesh 포인터 -> Model::meshes 인덱스를 기억해, 같은 지오메트리를 참조하는
// 여러 노드가 있어도(씬 그래프 내부 인스턴싱) Mesh를 한 번만 변환·저장하게 한다.
Node ConvertNode(const ufbx_node* ufbxNode, std::unordered_map<const ufbx_mesh*, size_t>& meshRegistry,
                  std::vector<std::shared_ptr<const ModelMesh>>& meshes)
{
    Node node;
    node.name = std::string(ufbxNode->name.data, ufbxNode->name.length);
    node.localTransform.position = ToVec3(ufbxNode->local_transform.translation);
    node.localTransform.rotation = ToQuaternion(ufbxNode->local_transform.rotation);
    node.localTransform.scale = ToVec3(ufbxNode->local_transform.scale);

    if (ufbxNode->mesh != nullptr)
    {
        const auto it = meshRegistry.find(ufbxNode->mesh);
        if (it != meshRegistry.end())
        {
            node.meshIndex = it->second;
        }
        else
        {
            const size_t newIndex = meshes.size();
            meshes.push_back(std::make_shared<const ModelMesh>(ConvertMesh(ufbxNode->mesh)));
            meshRegistry.emplace(ufbxNode->mesh, newIndex);
            node.meshIndex = newIndex;
        }
    }

    node.children.reserve(ufbxNode->children.count);
    for (size_t i = 0; i < ufbxNode->children.count; ++i)
    {
        node.children.push_back(ConvertNode(ufbxNode->children.data[i], meshRegistry, meshes));
    }

    return node;
}

}  // namespace

Model UfbxWrapper::Load(const std::string& path) const
{
    ufbx_load_opts opts;
    std::memset(&opts, 0, sizeof(opts));
    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(path.c_str(), &opts, &error);
    if (scene == nullptr)
    {
        throw std::runtime_error("UfbxWrapper: failed to load '" + path +
                                  "': " + std::string(error.description.data, error.description.length));
    }

    Model model;
    std::unordered_map<const ufbx_mesh*, size_t> meshRegistry;
    model.rootNode = ConvertNode(scene->root_node, meshRegistry, model.meshes);

    ufbx_free_scene(scene);
    return model;
}
