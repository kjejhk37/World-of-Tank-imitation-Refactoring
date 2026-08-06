#pragma once

#include "platform/entity_component/ComponentId.h"
#include "platform/entity_component/IComponent.h"
#include "platform/entity_component/UniqueId.h"

// Author: Claude
// Description: IComponent를 구현하는 편의 기반 클래스. 생성 시점에 ComponentId를 자동 발급해 저장한다.
// Input: (해당 없음 — 파생 클래스의 생성자에서 묵시적으로 호출됨)
// Output: GetId() — 생성 시 발급된 ComponentId
// Notes: 매 Component 구현체가 ID 저장·발급 로직을 반복하지 않도록 이 클래스를 상속하는 걸 권장한다
//        (직접 IComponent를 상속해도 되지만, 그 경우 ID 발급을 직접 구현해야 한다).
// Date: 2026-07-22
class ComponentBase : public IComponent
{
public:
    ComponentBase() : m_id(UniqueIdGenerator<ComponentId>::Next())
    {
    }

    ComponentId GetId() const override
    {
        return m_id;
    }

private:
    ComponentId m_id;
};
