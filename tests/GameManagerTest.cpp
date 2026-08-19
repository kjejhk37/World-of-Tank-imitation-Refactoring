#include <gtest/gtest.h>

#include "projects/game/GameManager.h"

TEST(GameManagerTest, ConstructionActivatesOneScene)
{
    GameManager gameManager;

    ASSERT_EQ(gameManager.GetSceneManager().GetActiveScenes().size(), 1u);
}

TEST(GameManagerTest, UpdateDoesNotCrash)
{
    GameManager gameManager;

    gameManager.Update(0.1f);

    SUCCEED();
}
