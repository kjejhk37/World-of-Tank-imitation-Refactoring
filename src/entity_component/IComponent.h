#pragma once

#include "entity_component/ComponentId.h"

// Author: Claude
// Description: 모든 Component가 구현해야 하는 최소 인터페이스. 자신을 식별하는 ComponentId 하나만 요구한다.
// Input: (해당 없음 — 인터페이스)
// Output: (해당 없음 — 인터페이스)
// Notes: 인터페이스를 최소로 유지해 ISP(인터페이스 분리 원칙)를 지킨다 — Component가 어떤 데이터를 갖든
//        이 인터페이스와는 무관하다. 실제 Component 구현체는 이 인터페이스를 직접 상속하기보다
//        ID 발급을 대신 처리해주는 ComponentBase를 상속하는 걸 권장한다.
// Date: 2026-07-22
class IComponent
{
public:
    virtual ~IComponent() = default;

    virtual ComponentId GetId() const = 0;
};
