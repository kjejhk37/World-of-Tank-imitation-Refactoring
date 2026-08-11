#include <gtest/gtest.h>

#include "projects/game/GameManager.h"
#include "projects/game/SceneSnapshotBuilder.h"

TEST(SceneSnapshotBuilderTest, BuildProducesOneMatrixPerEntity)
{
    GameManager gameManager(4);
    gameManager.Update(0.1f);

    InstanceSnapshot snapshot;
    SceneSnapshotBuilder::Build(gameManager.GetSceneManager(), snapshot);

    EXPECT_EQ(snapshot.worldMatrices.size(), 4u);
}

TEST(SceneSnapshotBuilderTest, BuildClearsPreviousSnapshotContent)
{
    GameManager gameManager(2);
    gameManager.Update(0.1f);

    InstanceSnapshot snapshot;
    snapshot.worldMatrices.resize(10);

    SceneSnapshotBuilder::Build(gameManager.GetSceneManager(), snapshot);
    EXPECT_EQ(snapshot.worldMatrices.size(), 2u);
}

TEST(SceneSnapshotBuilderTest, BuildReflectsSceneUpdatesOverTime)
{
    GameManager gameManager(1);

    gameManager.Update(0.1f);
    InstanceSnapshot firstSnapshot;
    SceneSnapshotBuilder::Build(gameManager.GetSceneManager(), firstSnapshot);

    gameManager.Update(0.5f);
    InstanceSnapshot secondSnapshot;
    SceneSnapshotBuilder::Build(gameManager.GetSceneManager(), secondSnapshot);

    bool anyComponentDiffers = false;
    for (int row = 0; row < 4 && !anyComponentDiffers; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            if (firstSnapshot.worldMatrices[0].m[row][col] != secondSnapshot.worldMatrices[0].m[row][col])
            {
                anyComponentDiffers = true;
                break;
            }
        }
    }
    EXPECT_TRUE(anyComponentDiffers);
}
