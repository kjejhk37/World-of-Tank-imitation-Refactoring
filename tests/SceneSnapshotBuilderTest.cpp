#include <gtest/gtest.h>

#include <cstddef>
#include <memory>

#include "platform/entity_component/IEntity.h"
#include "projects/game/SceneManager.h"
#include "projects/game/SceneSnapshotBuilder.h"
#include "projects/game/TransformComponent.h"

namespace
{
    // GameManager/IntroScene에 기대지 않고 SceneSnapshotBuilder만 독립적으로 검증하기 위한 테스트 전용
    // Scene - SceneManagerTest.cpp의 RecordingScene과 같은 패턴.
    class MovingEntityScene : public Scene
    {
    public:
        explicit MovingEntityScene(std::size_t entityCount) : m_entityCount(entityCount)
        {
        }

        void Start() override
        {
            for (std::size_t i = 0; i < m_entityCount; ++i)
            {
                AddComponent<TransformComponent>(CreateEntity());
            }
        }

        void Update(float deltaTime) override
        {
            m_elapsedSeconds += deltaTime;
            for (IEntity* entity : GetEntities())
            {
                GetComponent<TransformComponent>(*entity)->transform.position.y = m_elapsedSeconds;
            }
        }

    private:
        std::size_t m_entityCount;
        float m_elapsedSeconds = 0.0f;
    };

    std::unique_ptr<SceneManager> MakeActiveSceneManager(std::size_t entityCount)
    {
        auto sceneManager = std::make_unique<SceneManager>();
        sceneManager->Create("Test", std::make_unique<MovingEntityScene>(entityCount));
        sceneManager->Add("Test");
        return sceneManager;
    }
}

TEST(SceneSnapshotBuilderTest, BuildProducesOneMatrixPerEntity)
{
    auto sceneManager = MakeActiveSceneManager(4);
    sceneManager->Update(0.1f);

    InstanceSnapshot snapshot;
    SceneSnapshotBuilder::Build(*sceneManager, snapshot);

    EXPECT_EQ(snapshot.worldMatrices.size(), 4u);
}

TEST(SceneSnapshotBuilderTest, BuildClearsPreviousSnapshotContent)
{
    auto sceneManager = MakeActiveSceneManager(2);
    sceneManager->Update(0.1f);

    InstanceSnapshot snapshot;
    snapshot.worldMatrices.resize(10);

    SceneSnapshotBuilder::Build(*sceneManager, snapshot);
    EXPECT_EQ(snapshot.worldMatrices.size(), 2u);
}

TEST(SceneSnapshotBuilderTest, BuildReflectsSceneUpdatesOverTime)
{
    auto sceneManager = MakeActiveSceneManager(1);

    sceneManager->Update(0.1f);
    InstanceSnapshot firstSnapshot;
    SceneSnapshotBuilder::Build(*sceneManager, firstSnapshot);

    sceneManager->Update(0.5f);
    InstanceSnapshot secondSnapshot;
    SceneSnapshotBuilder::Build(*sceneManager, secondSnapshot);

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

TEST(SceneSnapshotBuilderTest, BuildProducesEmptySnapshotWhenSceneHasNoEntities)
{
    auto sceneManager = MakeActiveSceneManager(0);
    sceneManager->Update(0.1f);

    InstanceSnapshot snapshot;
    SceneSnapshotBuilder::Build(*sceneManager, snapshot);

    EXPECT_TRUE(snapshot.worldMatrices.empty());
}
