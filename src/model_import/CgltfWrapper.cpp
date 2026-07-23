#include "model_import/CgltfWrapper.h"

#include <cmath>
#include <cstring>
#include <stdexcept>
#include <unordered_map>

#include "math/MathConstants.h"
#include "math/Quaternion.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

namespace
{

// glTF의 matrix(has_matrix)는 column-major 4x4다. 정점 데이터에 shear가 없다고 가정하고
// 이동/스케일/회전으로 분해한다(이 사이클의 명시적 단순화 - Notes 참고).
// 한계: 행렬식이 음수인(미러링/반사가 포함된) 변환은 이 방식으로 구분할 수 없다 - 컬럼을 그대로
// 정규화해 회전으로 취급하므로, 반사가 있는 노드는 반사가 빠진(잘못된) 회전으로 조용히 변환된다.
// 이런 노드가 있는 에셋은 이번 사이클 스코프 밖이며, 필요해지면 행렬식 부호를 확인해 별도 처리해야 한다.
Quaternion QuaternionFromOrthonormalBasis(const Vec3& axisX, const Vec3& axisY, const Vec3& axisZ)
{
    const float trace = axisX.x + axisY.y + axisZ.z;
    if (trace > 0.0f)
    {
        const float s = std::sqrt(trace + 1.0f) * 2.0f;
        return Quaternion((axisY.z - axisZ.y) / s, (axisZ.x - axisX.z) / s, (axisX.y - axisY.x) / s, 0.25f * s);
    }
    if (axisX.x > axisY.y && axisX.x > axisZ.z)
    {
        const float s = std::sqrt(1.0f + axisX.x - axisY.y - axisZ.z) * 2.0f;
        return Quaternion(0.25f * s, (axisY.x + axisX.y) / s, (axisZ.x + axisX.z) / s, (axisY.z - axisZ.y) / s);
    }
    if (axisY.y > axisZ.z)
    {
        const float s = std::sqrt(1.0f + axisY.y - axisX.x - axisZ.z) * 2.0f;
        return Quaternion((axisY.x + axisX.y) / s, 0.25f * s, (axisZ.y + axisY.z) / s, (axisZ.x - axisX.z) / s);
    }
    const float s = std::sqrt(1.0f + axisZ.z - axisX.x - axisY.y) * 2.0f;
    return Quaternion((axisZ.x + axisX.z) / s, (axisZ.y + axisY.z) / s, 0.25f * s, (axisX.y - axisY.x) / s);
}

Transform DecomposeMatrix(const cgltf_float matrix[16])
{
    const Vec3 col0(matrix[0], matrix[1], matrix[2]);
    const Vec3 col1(matrix[4], matrix[5], matrix[6]);
    const Vec3 col2(matrix[8], matrix[9], matrix[10]);

    const float scaleX = col0.Length();
    const float scaleY = col1.Length();
    const float scaleZ = col2.Length();

    const Vec3 axisX = scaleX > MathConstants::kEpsilon ? col0 / scaleX : Vec3(1.0f, 0.0f, 0.0f);
    const Vec3 axisY = scaleY > MathConstants::kEpsilon ? col1 / scaleY : Vec3(0.0f, 1.0f, 0.0f);
    const Vec3 axisZ = scaleZ > MathConstants::kEpsilon ? col2 / scaleZ : Vec3(0.0f, 0.0f, 1.0f);

    Transform transform;
    transform.position = Vec3(matrix[12], matrix[13], matrix[14]);
    transform.rotation = QuaternionFromOrthonormalBasis(axisX, axisY, axisZ);
    transform.scale = Vec3(scaleX, scaleY, scaleZ);
    return transform;
}

Transform ConvertTransform(const cgltf_node* n)
{
    if (n->has_matrix)
    {
        return DecomposeMatrix(n->matrix);
    }

    Transform transform;
    transform.position = n->has_translation ? Vec3(n->translation[0], n->translation[1], n->translation[2])
                                             : Vec3::Zero();
    transform.rotation = n->has_rotation
                              ? Quaternion(n->rotation[0], n->rotation[1], n->rotation[2], n->rotation[3])
                              : Quaternion::Identity();
    transform.scale = n->has_scale ? Vec3(n->scale[0], n->scale[1], n->scale[2]) : Vec3::One();
    return transform;
}

ModelMesh ConvertMesh(const cgltf_mesh* cgltfMesh)
{
    ModelMesh mesh;
    mesh.uvChannels.resize(1);

    for (size_t p = 0; p < cgltfMesh->primitives_count; ++p)
    {
        const cgltf_primitive& primitive = cgltfMesh->primitives[p];
        if (primitive.type != cgltf_primitive_type_triangles)
        {
            continue;
        }

        const cgltf_accessor* positionAccessor = nullptr;
        const cgltf_accessor* normalAccessor = nullptr;
        const cgltf_accessor* uvAccessor = nullptr;
        for (size_t a = 0; a < primitive.attributes_count; ++a)
        {
            const cgltf_attribute& attribute = primitive.attributes[a];
            if (attribute.type == cgltf_attribute_type_position)
            {
                positionAccessor = attribute.data;
            }
            else if (attribute.type == cgltf_attribute_type_normal)
            {
                normalAccessor = attribute.data;
            }
            else if (attribute.type == cgltf_attribute_type_texcoord && attribute.index == 0)
            {
                uvAccessor = attribute.data;
            }
        }

        if (positionAccessor == nullptr)
        {
            continue;
        }

        const uint32_t vertexOffset = static_cast<uint32_t>(mesh.positions.size());
        for (size_t v = 0; v < positionAccessor->count; ++v)
        {
            float pos[3] = {0.0f, 0.0f, 0.0f};
            cgltf_accessor_read_float(positionAccessor, v, pos, 3);
            mesh.positions.push_back(Vec3(pos[0], pos[1], pos[2]));

            Vec3 normal = Vec3::Zero();
            if (normalAccessor != nullptr)
            {
                float n[3] = {0.0f, 0.0f, 0.0f};
                cgltf_accessor_read_float(normalAccessor, v, n, 3);
                normal = Vec3(n[0], n[1], n[2]);
            }
            mesh.normals.push_back(normal);

            Vec2 uv(0.0f, 0.0f);
            if (uvAccessor != nullptr)
            {
                float t[2] = {0.0f, 0.0f};
                cgltf_accessor_read_float(uvAccessor, v, t, 2);
                uv = Vec2(t[0], t[1]);
            }
            mesh.uvChannels[0].push_back(uv);
        }

        if (primitive.indices != nullptr)
        {
            for (size_t i = 0; i < primitive.indices->count; ++i)
            {
                const cgltf_size index = cgltf_accessor_read_index(primitive.indices, i);
                mesh.indices.push_back(static_cast<uint32_t>(index) + vertexOffset);
            }
        }
        else
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(positionAccessor->count); ++i)
            {
                mesh.indices.push_back(i + vertexOffset);
            }
        }

        if (!mesh.materialRef.has_value() && primitive.material != nullptr && primitive.material->name != nullptr)
        {
            mesh.materialRef = std::string(primitive.material->name);
        }
    }

    return mesh;
}

// meshRegistry는 cgltf_mesh 포인터 -> Model::meshes 인덱스를 기억해, 같은 지오메트리를 참조하는
// 여러 노드가 있어도(씬 그래프 내부 인스턴싱) Mesh를 한 번만 변환·저장하게 한다.
Node ConvertNode(const cgltf_node* n, std::unordered_map<const cgltf_mesh*, size_t>& meshRegistry,
                  std::vector<std::shared_ptr<const ModelMesh>>& meshes)
{
    Node node;
    node.name = n->name != nullptr ? std::string(n->name) : std::string();
    node.localTransform = ConvertTransform(n);

    if (n->mesh != nullptr)
    {
        const auto it = meshRegistry.find(n->mesh);
        if (it != meshRegistry.end())
        {
            node.meshIndex = it->second;
        }
        else
        {
            const size_t newIndex = meshes.size();
            meshes.push_back(std::make_shared<const ModelMesh>(ConvertMesh(n->mesh)));
            meshRegistry.emplace(n->mesh, newIndex);
            node.meshIndex = newIndex;
        }
    }

    node.children.reserve(n->children_count);
    for (size_t i = 0; i < n->children_count; ++i)
    {
        node.children.push_back(ConvertNode(n->children[i], meshRegistry, meshes));
    }

    return node;
}

}  // namespace

Model CgltfWrapper::Load(const std::string& path) const
{
    cgltf_options options;
    std::memset(&options, 0, sizeof(options));
    cgltf_data* data = nullptr;

    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
    if (result != cgltf_result_success)
    {
        throw std::runtime_error("CgltfWrapper: failed to parse '" + path + "' (cgltf_result=" +
                                  std::to_string(static_cast<int>(result)) + ")");
    }

    result = cgltf_load_buffers(&options, data, path.c_str());
    if (result != cgltf_result_success)
    {
        cgltf_free(data);
        throw std::runtime_error("CgltfWrapper: failed to load buffers for '" + path + "' (cgltf_result=" +
                                  std::to_string(static_cast<int>(result)) + ")");
    }

    Model model;
    std::unordered_map<const cgltf_mesh*, size_t> meshRegistry;

    // glTF 씬은 루트 노드가 여러 개일 수 있으므로, 이 프로젝트의 단일 루트 전제를 맞추기 위해
    // 항상 identity Transform의 합성 루트 노드로 감싼다(루트가 하나뿐이어도 동일하게 처리).
    model.rootNode.name = "root";
    if (data->scene != nullptr)
    {
        model.rootNode.children.reserve(data->scene->nodes_count);
        for (size_t i = 0; i < data->scene->nodes_count; ++i)
        {
            model.rootNode.children.push_back(ConvertNode(data->scene->nodes[i], meshRegistry, model.meshes));
        }
    }
    else
    {
        for (size_t i = 0; i < data->nodes_count; ++i)
        {
            if (data->nodes[i].parent == nullptr)
            {
                model.rootNode.children.push_back(ConvertNode(&data->nodes[i], meshRegistry, model.meshes));
            }
        }
    }

    cgltf_free(data);
    return model;
}
