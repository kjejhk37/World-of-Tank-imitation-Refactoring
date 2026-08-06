#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "platform/entity_component/IEntity.h"

// Author: Claude
// Description: IEntity의 구현체. 자신이 소유한 Component를 ComponentId로 저장하고, 타입별 조회를 위한
//              보조 인덱스(type_index -> ComponentId 목록)를 함께 유지한다.
// Input: (해당 없음 — 생성자는 EntityManager가 호출)
// Output: (IEntity 인터페이스 참고)
// Notes: Component의 실제 소유권은 m_components(ComponentId -> unique_ptr<IComponent>)에 있고,
//        m_componentsByType은 그 Component를 가리키는 ComponentId만 보관하는 비소유 보조 인덱스다.
//        Entity가 소멸하면 m_components가 소멸하며 소유한 Component가 전부 자동 소멸한다(RAII) —
//        Component는 Entity를 통해서만 생성·소멸된다는 원칙이 여기서 자연히 성립한다.
//        이 클래스는 단일 스레드에서의 호출만 지원한다 — 동시 접근에 대한 락은 없다
//        (docs/brainstorming/Component_패턴_기본구조_20260722_2217.md Details 7 참고).
// Date: 2026-07-22
class Entity : public IEntity
{
public:
    Entity();

    EntityId GetId() const override;
    IComponent& AddComponent(std::unique_ptr<IComponent> component) override;
    bool RemoveComponent(ComponentId id) override;
    IComponent* FindComponent(std::type_index type) const override;
    std::vector<IComponent*> FindComponents(std::type_index type) const override;

private:
    EntityId m_id;
    std::unordered_map<ComponentId, std::unique_ptr<IComponent>> m_components;
    std::unordered_map<std::type_index, std::vector<ComponentId>> m_componentsByType;
};
