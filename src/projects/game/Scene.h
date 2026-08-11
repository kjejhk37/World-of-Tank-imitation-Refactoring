#pragma once

#include <vector>

#include "platform/entity_component/EntityManager.h"
#include "platform/entity_component/IEntity.h"

// Author: Claude
// Description: 씬 하나의 생명주기(Start/Update/End)와 그 씬이 소유한 Entity 집합을 표현하는 추상 기반 클래스.
//              원본 WOT-master의 Scene과 달리 PreRender/Render/PostRender/GUIRender는 갖지 않는다 —
//              렌더링은 이미 IRenderer/렌더 스레드가 InstanceSnapshot을 통해 전담하므로, Scene은
//              engine 스레드에서 도는 로직(Update)만 책임진다.
// Input: (해당 없음 — 각 메서드 참고)
// Output: (해당 없음 — 각 메서드 참고)
// Notes: EntityManager는 Entity의 생성/소멸 수명주기만 담당하고(원래 책임 그대로) 전체 열거 기능이
//        없다(platform/entity_component/EntityManager.h 참고 — 전역 열거용 Registry를 두지 않는 게
//        그 클래스의 기존 설계 원칙이다). 그래서 이 클래스는 CreateEntity()로 만든 Entity를
//        m_entities에도 같이 등록해 스스로 열거 목록을 유지한다 — platform은 이 때문에 수정하지 않는다.
// Date: 2026-08-11
class Scene
{
public:
    virtual ~Scene() = default;

    virtual void Start() {}
    virtual void Update(float deltaTime) = 0;
    virtual void End() {}

    const std::vector<IEntity*>& GetEntities() const
    {
        return m_entities;
    }

protected:
    IEntity& CreateEntity()
    {
        IEntity& entity = m_entityManager.CreateEntity();
        m_entities.push_back(&entity);
        return entity;
    }

private:
    EntityManager m_entityManager;
    std::vector<IEntity*> m_entities;
};
