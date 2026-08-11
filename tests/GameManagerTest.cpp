#include <gtest/gtest.h>

#include "platform/entity_component/IEntity.h"
#include "projects/game/GameManager.h"
#include "projects/game/TransformComponent.h"

TEST(GameManagerTest, ConstructionActivatesOneSceneWithRequestedInstanceCount)
{
    GameManager gameManager(5);

    ASSERT_EQ(gameManager.GetSceneManager().GetActiveScenes().size(), 1u);
    EXPECT_EQ(gameManager.GetSceneManager().GetActiveScenes().front()->GetEntities().size(), 5u);
}

TEST(GameManagerTest, UpdateAdvancesTheActiveScene)
{
    GameManager gameManager(1);
    IEntity* entity = gameManager.GetSceneManager().GetActiveScenes().front()->GetEntities()[0];

    gameManager.Update(0.1f);
    const float firstY = GetComponent<TransformComponent>(*entity)->transform.position.y;

    gameManager.Update(0.5f);
    const float secondY = GetComponent<TransformComponent>(*entity)->transform.position.y;

    EXPECT_NE(firstY, secondY);
}
