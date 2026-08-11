#include "projects/game/GameManager.h"

#include <memory>

#include "projects/game/PrototypeScene.h"

namespace
{
    constexpr const char* kPrototypeSceneKey = "Prototype";
}

GameManager::GameManager(std::size_t instanceCount)
{
    m_sceneManager.Create(kPrototypeSceneKey, std::make_unique<PrototypeScene>(instanceCount));
    m_sceneManager.Add(kPrototypeSceneKey);
}

void GameManager::Update(float deltaTime)
{
    m_sceneManager.Update(deltaTime);
}

const SceneManager& GameManager::GetSceneManager() const
{
    return m_sceneManager;
}
