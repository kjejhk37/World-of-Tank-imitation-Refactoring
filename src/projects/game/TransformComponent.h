#pragma once

#include "platform/entity_component/ComponentBase.h"
#include "platform/math/Transform.h"

// Author: Claude
// Description: 위치/회전/스케일(TRS)을 갖는 최소 데이터 Component. platform/math::Transform을 그대로 감싼다.
// Input: (해당 없음 — 생성자는 ComponentBase의 기본 생성자로 ComponentId만 발급)
// Output: (해당 없음 — transform 멤버에 직접 접근)
// Notes: 이번 사이클은 씬 전환·생명주기 뼈대 검증이 목표라 Mesh/Material 등은 아직 없다 —
//        Entity 하나가 이 Component 하나만 갖는 최소 형태로 EC 통합을 검증한다.
// Date: 2026-08-11
class TransformComponent : public ComponentBase
{
public:
    Transform transform;
};
