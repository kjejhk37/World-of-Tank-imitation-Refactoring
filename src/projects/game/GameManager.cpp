#include "projects/game/GameManager.h"

#include <memory>

#include "projects/game/IntroScene.h"

namespace
{
    constexpr const char* kIntroSceneKey = "Intro";
}

GameManager::GameManager()
{
    m_sceneManager.Create(kIntroSceneKey, std::make_unique<IntroScene>());
    m_sceneManager.Add(kIntroSceneKey);
}

void GameManager::Update(float deltaTime)
{
    m_sceneManager.Update(deltaTime);
}

const SceneManager& GameManager::GetSceneManager() const
{
    return m_sceneManager;
}
