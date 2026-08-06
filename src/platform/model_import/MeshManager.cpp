#include "platform/model_import/MeshManager.h"

#include <utility>

MeshManager::MeshManager(std::unique_ptr<IMeshCachePolicy> cachePolicy,
                         std::unique_ptr<IMeshDeduplicator> deduplicator)
    : m_cachePolicy(std::move(cachePolicy)), m_deduplicator(std::move(deduplicator))
{
}

std::shared_ptr<const Model> MeshManager::GetOrLoad(const std::string& key, const std::function<Model()>& loader)
{
    std::shared_ptr<const Model> cached = m_cachePolicy->TryGet(key);
    if (cached != nullptr)
    {
        return cached;
    }

    Model freshModel = loader();

    if (m_deduplicator != nullptr)
    {
        for (std::shared_ptr<const ModelMesh>& meshPtr : freshModel.meshes)
        {
            meshPtr = m_deduplicator->FindOrRegister(meshPtr);
        }
    }

    auto model = std::make_shared<const Model>(std::move(freshModel));
    m_cachePolicy->Store(key, model);
    return model;
}

void MeshManager::Unload(const std::string& key)
{
    m_cachePolicy->Unload(key);
}
