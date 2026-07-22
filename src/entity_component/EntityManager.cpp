#include "entity_component/EntityManager.h"

IEntity& EntityManager::CreateEntity()
{
    auto entity = std::make_unique<Entity>();
    IEntity& ref = *entity;
    m_entities.emplace(ref.GetId(), std::move(entity));
    return ref;
}

void EntityManager::DestroyEntity(EntityId id)
{
    m_entities.erase(id);
}

IEntity* EntityManager::FindEntity(EntityId id) const
{
    const auto it = m_entities.find(id);
    return it == m_entities.end() ? nullptr : it->second.get();
}
