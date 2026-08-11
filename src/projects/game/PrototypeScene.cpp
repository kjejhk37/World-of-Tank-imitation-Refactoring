#include "projects/game/PrototypeScene.h"

#include <cmath>

#include "platform/entity_component/IEntity.h"
#include "platform/math/Quaternion.h"
#include "platform/math/Vec3.h"
#include "projects/game/TransformComponent.h"

namespace
{
    constexpr float kGridSpacing = 2.5f;
    constexpr float kBobAmplitude = 0.5f;
    constexpr float kDegreesPerSecond = 45.0f;
}

PrototypeScene::PrototypeScene(std::size_t instanceCount) : m_instanceCount(instanceCount)
{
}

void PrototypeScene::Start()
{
    for (std::size_t i = 0; i < m_instanceCount; ++i)
    {
        IEntity& entity = CreateEntity();
        AddComponent<TransformComponent>(entity);
    }
}

void PrototypeScene::Update(float deltaTime)
{
    m_elapsedSeconds += deltaTime;

    const std::vector<IEntity*>& entities = GetEntities();
    const int gridSize =
        entities.empty() ? 0 : static_cast<int>(std::ceil(std::sqrt(static_cast<double>(entities.size()))));

    for (std::size_t i = 0; i < entities.size(); ++i)
    {
        TransformComponent* transformComponent = GetComponent<TransformComponent>(*entities[i]);
        if (transformComponent == nullptr)
        {
            continue;
        }

        const int row = gridSize == 0 ? 0 : static_cast<int>(i) / gridSize;
        const int col = gridSize == 0 ? 0 : static_cast<int>(i) % gridSize;

        transformComponent->transform.position =
            Vec3(static_cast<float>(col) * kGridSpacing,
                 std::sin(m_elapsedSeconds + static_cast<float>(i)) * kBobAmplitude,
                 static_cast<float>(row) * kGridSpacing);
        transformComponent->transform.rotation = Quaternion::FromAxisAngle(
            Vec3(0.0f, 1.0f, 0.0f), m_elapsedSeconds * kDegreesPerSecond + static_cast<float>(i) * 10.0f);
    }
}
