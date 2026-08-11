#include <gtest/gtest.h>

#include "platform/entity_component/IEntity.h"
#include "projects/game/PrototypeScene.h"
#include "projects/game/TransformComponent.h"

TEST(PrototypeSceneTest, EntitiesAreEmptyBeforeStart)
{
    PrototypeScene scene(4);
    EXPECT_TRUE(scene.GetEntities().empty());
}

TEST(PrototypeSceneTest, StartCreatesRequestedEntityCount)
{
    PrototypeScene scene(4);
    scene.Start();

    EXPECT_EQ(scene.GetEntities().size(), 4u);
}

TEST(PrototypeSceneTest, EveryEntityHasTransformComponent)
{
    PrototypeScene scene(3);
    scene.Start();

    for (IEntity* entity : scene.GetEntities())
    {
        EXPECT_NE(GetComponent<TransformComponent>(*entity), nullptr);
    }
}

TEST(PrototypeSceneTest, UpdateChangesTransformOverTime)
{
    PrototypeScene scene(1);
    scene.Start();

    scene.Update(0.1f);
    const float firstY = GetComponent<TransformComponent>(*scene.GetEntities()[0])->transform.position.y;

    scene.Update(0.5f);
    const float secondY = GetComponent<TransformComponent>(*scene.GetEntities()[0])->transform.position.y;

    EXPECT_NE(firstY, secondY);
}
