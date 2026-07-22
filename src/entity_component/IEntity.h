#pragma once

#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include "entity_component/ComponentId.h"
#include "entity_component/EntityId.h"
#include "entity_component/IComponent.h"

// Author: Claude
// Description: Entity가 Component를 저장·조회하기 위해 구현해야 하는 인터페이스.
// Input: (해당 없음 — 인터페이스)
// Output: (해당 없음 — 인터페이스)
// Notes: C++은 가상 함수에 템플릿을 쓸 수 없어, 인터페이스 자체는 std::type_index 기반의 non-template
//        가상 함수만 선언한다. 타입 안전한 GetComponent<T>()/AddComponent<T>() 등은 이 헤더 하단의
//        free template 함수(예: std::get<T>가 std::variant의 멤버가 아니라 free function인 것과 같은 이유)로 제공한다.
//        AddComponent는 내부에서 typeid(*component)(런타임 동적 타입)로 타입 인덱스를 갱신하므로,
//        조회 시 typeid(T)(호출부의 정적 타입)와 일치하려면 항상 컴포넌트의 가장 파생된 구체 클래스로
//        Add/Get해야 한다 (중간 기반 클래스로 조회하면 못 찾는다).
// Date: 2026-07-22
class IEntity
{
public:
    virtual ~IEntity() = default;

    virtual EntityId GetId() const = 0;
    virtual IComponent& AddComponent(std::unique_ptr<IComponent> component) = 0;
    virtual bool RemoveComponent(ComponentId id) = 0;
    virtual IComponent* FindComponent(std::type_index type) const = 0;
    virtual std::vector<IComponent*> FindComponents(std::type_index type) const = 0;
};

// Author: Claude
// Description: entity에 T 타입 Component를 새로 생성해 추가하는 타입 안전 편의 함수.
// Input: entity — Component를 추가할 대상 / args — T의 생성자에 전달할 인자
// Output: 새로 추가된 Component에 대한 T& 참조
// Notes: T는 반드시 IComponent를 상속해야 한다 (static_assert로 강제).
// Date: 2026-07-22
template <typename T, typename... Args>
T& AddComponent(IEntity& entity, Args&&... args)
{
    static_assert(std::is_base_of<IComponent, T>::value, "T must derive from IComponent");

    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    T& ref = *component;
    entity.AddComponent(std::move(component));
    return ref;
}

// Author: Claude
// Description: entity가 가진 T 타입 Component 중 첫 번째를 찾는 타입 안전 편의 함수.
// Input: entity — 조회 대상
// Output: 찾으면 T*, 없으면 nullptr
// Notes: 같은 타입 Component가 여러 개 있어도 그중 하나만 반환한다 — 전부 필요하면 GetComponents<T>()를 쓴다.
//        dynamic_cast를 쓰는 이유: type_index 기반 내부 색인이 항상 정확하다는 전제로 static_cast를 쓸 수도
//        있지만, 그 불변조건이 다른 IEntity 구현체 등으로 인해 깨지는 경우에도 조용한 UB 대신 안전하게
//        nullptr을 반환하도록 방어적으로 작성했다.
// Date: 2026-07-22
template <typename T>
T* GetComponent(const IEntity& entity)
{
    return dynamic_cast<T*>(entity.FindComponent(std::type_index(typeid(T))));
}

// Author: Claude
// Description: entity가 가진 T 타입 Component를 전부 찾는 타입 안전 편의 함수.
// Input: entity — 조회 대상
// Output: T* 목록 (없으면 빈 vector)
// Notes: 신체 부위별 Collider처럼 같은 타입 Component를 여러 개 다루는 시나리오를 위한 API다.
// Date: 2026-07-22
template <typename T>
std::vector<T*> GetComponents(const IEntity& entity)
{
    std::vector<T*> result;
    for (IComponent* component : entity.FindComponents(std::type_index(typeid(T))))
    {
        result.push_back(dynamic_cast<T*>(component));
    }
    return result;
}
