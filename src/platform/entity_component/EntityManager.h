#pragma once

#include <memory>
#include <unordered_map>

#include "platform/entity_component/Entity.h"

// Author: Claude
// Description: Entity의 생성·조회·파괴 수명 주기를 전담 관리한다. Component 자체는 관리하지 않는다 —
//              Component의 생성·소멸은 그 Component를 소유한 Entity가 전담한다(Entity.h 참고).
// Input: (해당 없음 — 각 메서드 참고)
// Output: (각 메서드 참고)
// Notes: Component uuid로 Entity를 거치지 않고 전역 조회하는 별도 Registry는 두지 않는다 — 실제 호출
//        패턴이 항상 Entity를 먼저 거치므로 지금 만들면 YAGNI라는 브레인스토밍 결론에 따른 것이다
//        (docs/brainstorming/Component_패턴_기본구조_20260722_2217.md Q4 참고).
//        이 클래스는 단일 스레드에서의 호출만 지원한다 — 동시 접근에 대한 락은 없다(Details 7 참고).
// Date: 2026-07-22
class EntityManager
{
public:
    IEntity& CreateEntity();
    void DestroyEntity(EntityId id);
    IEntity* FindEntity(EntityId id) const;

private:
    std::unordered_map<EntityId, std::unique_ptr<IEntity>> m_entities;
};
