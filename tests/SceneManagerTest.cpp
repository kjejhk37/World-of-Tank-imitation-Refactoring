#include <gtest/gtest.h>

#include <memory>

#include "projects/game/SceneManager.h"

namespace
{
    class RecordingScene : public Scene
    {
    public:
        int startCount = 0;
        int updateCount = 0;
        int endCount = 0;
        float lastDeltaTime = 0.0f;

        void Start() override
        {
            ++startCount;
        }

        void Update(float deltaTime) override
        {
            ++updateCount;
            lastDeltaTime = deltaTime;
        }

        void End() override
        {
            ++endCount;
        }
    };
}

TEST(SceneManagerTest, AddCallsStartExactlyOnceEvenIfCalledTwice)
{
    SceneManager sceneManager;
    auto scene = std::make_unique<RecordingScene>();
    RecordingScene* rawScene = scene.get();
    sceneManager.Create("Test", std::move(scene));

    EXPECT_EQ(rawScene->startCount, 0);

    sceneManager.Add("Test");
    EXPECT_EQ(rawScene->startCount, 1);

    sceneManager.Add("Test");
    EXPECT_EQ(rawScene->startCount, 1);
}

TEST(SceneManagerTest, UpdateOnlyDelegatesToActiveScenes)
{
    SceneManager sceneManager;
    auto scene = std::make_unique<RecordingScene>();
    RecordingScene* rawScene = scene.get();
    sceneManager.Create("Test", std::move(scene));

    sceneManager.Update(0.1f);
    EXPECT_EQ(rawScene->updateCount, 0);

    sceneManager.Add("Test");
    sceneManager.Update(0.25f);
    EXPECT_EQ(rawScene->updateCount, 1);
    EXPECT_FLOAT_EQ(rawScene->lastDeltaTime, 0.25f);
}

TEST(SceneManagerTest, RemoveCallsEndAndStopsFurtherUpdates)
{
    SceneManager sceneManager;
    auto scene = std::make_unique<RecordingScene>();
    RecordingScene* rawScene = scene.get();
    sceneManager.Create("Test", std::move(scene));
    sceneManager.Add("Test");

    sceneManager.Remove("Test");
    EXPECT_EQ(rawScene->endCount, 1);

    sceneManager.Update(0.1f);
    EXPECT_EQ(rawScene->updateCount, 0);
}

TEST(SceneManagerTest, RemoveUnknownKeyDoesNotCrash)
{
    SceneManager sceneManager;
    sceneManager.Remove("Nonexistent");
    SUCCEED();
}

TEST(SceneManagerTest, GetActiveScenesReflectsAddAndRemove)
{
    SceneManager sceneManager;
    sceneManager.Create("Test", std::make_unique<RecordingScene>());
    EXPECT_TRUE(sceneManager.GetActiveScenes().empty());

    sceneManager.Add("Test");
    EXPECT_EQ(sceneManager.GetActiveScenes().size(), 1u);

    sceneManager.Remove("Test");
    EXPECT_TRUE(sceneManager.GetActiveScenes().empty());
}
