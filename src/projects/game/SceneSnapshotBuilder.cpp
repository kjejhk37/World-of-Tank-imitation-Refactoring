#include "projects/game/SceneSnapshotBuilder.h"

#include "platform/entity_component/IEntity.h"
#include "projects/game/TransformComponent.h"

void SceneSnapshotBuilder::Build(const SceneManager& sceneManager, InstanceSnapshot& outSnapshot)
{
    outSnapshot.worldMatrices.clear();

    for (const Scene* scene : sceneManager.GetActiveScenes())
    {
        for (IEntity* entity : scene->GetEntities())
        {
            const TransformComponent* transformComponent = GetComponent<TransformComponent>(*entity);
            if (transformComponent == nullptr)
            {
                continue;
            }

            outSnapshot.worldMatrices.push_back(transformComponent->transform.ToMatrix());
        }
    }
}
