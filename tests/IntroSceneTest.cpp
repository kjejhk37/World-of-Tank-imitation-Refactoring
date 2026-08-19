#include <gtest/gtest.h>

#include "projects/game/IntroScene.h"

TEST(IntroSceneTest, HasNoEntitiesBeforeWidgetsAreAdded)
{
    IntroScene scene;
    EXPECT_TRUE(scene.GetEntities().empty());
}

TEST(IntroSceneTest, UpdateDoesNotCrash)
{
    IntroScene scene;
    scene.Update(0.1f);

    SUCCEED();
}
