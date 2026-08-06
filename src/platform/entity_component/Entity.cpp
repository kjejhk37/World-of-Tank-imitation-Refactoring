#include "platform/entity_component/Entity.h"

#include <algorithm>
#include <cassert>
#include <typeindex>

#include "platform/entity_component/UniqueId.h"

Entity::Entity() : m_id(UniqueIdGenerator<EntityId>::Next())
{
}

EntityId Entity::GetId() const
{
    return m_id;
}

IComponent& Entity::AddComponent(std::unique_ptr<IComponent> component)
{
    assert(component != nullptr && "AddComponent requires a non-null component");

    const ComponentId id = component->GetId();
    const std::type_index type(typeid(*component));

    IComponent& ref = *component;
    m_components.emplace(id, std::move(component));
    m_componentsByType[type].push_back(id);
    return ref;
}

bool Entity::RemoveComponent(ComponentId id)
{
    const auto componentIt = m_components.find(id);
    if (componentIt == m_components.end())
    {
        return false;
    }

    const std::type_index type(typeid(*componentIt->second));
    auto typeIt = m_componentsByType.find(type);
    if (typeIt != m_componentsByType.end())
    {
        auto& ids = typeIt->second;
        ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
        if (ids.empty())
        {
            m_componentsByType.erase(typeIt);
        }
    }

    m_components.erase(componentIt);
    return true;
}

IComponent* Entity::FindComponent(std::type_index type) const
{
    const auto typeIt = m_componentsByType.find(type);
    if (typeIt == m_componentsByType.end() || typeIt->second.empty())
    {
        return nullptr;
    }

    const auto componentIt = m_components.find(typeIt->second.front());
    return componentIt == m_components.end() ? nullptr : componentIt->second.get();
}

std::vector<IComponent*> Entity::FindComponents(std::type_index type) const
{
    std::vector<IComponent*> result;

    const auto typeIt = m_componentsByType.find(type);
    if (typeIt == m_componentsByType.end())
    {
        return result;
    }

    result.reserve(typeIt->second.size());
    for (ComponentId id : typeIt->second)
    {
        const auto componentIt = m_components.find(id);
        if (componentIt != m_components.end())
        {
            result.push_back(componentIt->second.get());
        }
    }
    return result;
}
