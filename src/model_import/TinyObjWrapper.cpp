#include "model_import/TinyObjWrapper.h"

#include <stdexcept>

#include "tiny_obj_loader.h"

ModelMesh TinyObjWrapper::Load(const std::string& path) const
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    const bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!ok)
    {
        throw std::runtime_error("TinyObjWrapper: failed to load '" + path + "': " + err);
    }

    ModelMesh mesh;
    mesh.uvChannels.resize(1);

    for (const tinyobj::shape_t& shape : shapes)
    {
        int faceMaterialId = -1;
        for (size_t i = 0; i < shape.mesh.indices.size(); ++i)
        {
            const tinyobj::index_t& index = shape.mesh.indices[i];

            const Vec3 position(
                attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 0],
                attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 1],
                attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 2]);
            mesh.positions.push_back(position);

            Vec3 normal = Vec3::Zero();
            if (index.normal_index >= 0)
            {
                normal = Vec3(
                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 0],
                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1],
                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2]);
            }
            mesh.normals.push_back(normal);

            Vec2 uv(0.0f, 0.0f);
            if (index.texcoord_index >= 0)
            {
                uv = Vec2(
                    attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 0],
                    attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1]);
            }
            mesh.uvChannels[0].push_back(uv);

            mesh.indices.push_back(static_cast<uint32_t>(mesh.indices.size()));

            const size_t faceIndex = i / 3;
            if (faceMaterialId < 0 && faceIndex < shape.mesh.material_ids.size())
            {
                faceMaterialId = shape.mesh.material_ids[faceIndex];
            }
        }

        if (!mesh.materialRef.has_value() && faceMaterialId >= 0 &&
            static_cast<size_t>(faceMaterialId) < materials.size())
        {
            mesh.materialRef = materials[static_cast<size_t>(faceMaterialId)].name;
        }
    }

    return mesh;
}
