#include "projects/game/SceneManager.h"

#include <algorithm>

void SceneManager::Create(const std::string& key, std::unique_ptr<Scene> scene)
{
    if (m_scenes.count(key) > 0)
    {
        return;
    }

    m_scenes.emplace(key, std::move(scene));
}

void SceneManager::Add(const std::string& key)
{
    const auto it = m_scenes.find(key);
    if (it == m_scenes.end())
    {
        return;
    }

    Scene* scene = it->second.get();
    if (std::find(m_activeScenes.begin(), m_activeScenes.end(), scene) != m_activeScenes.end())
    {
        return;
    }

    m_activeScenes.push_back(scene);
    scene->Start();
}

void SceneManager::Remove(const std::string& key)
{
    const auto it = m_scenes.find(key);
    if (it == m_scenes.end())
    {
        return;
    }

    Scene* scene = it->second.get();
    const auto activeIt = std::find(m_activeScenes.begin(), m_activeScenes.end(), scene);
    if (activeIt == m_activeScenes.end())
    {
        return;
    }

    scene->End();
    m_activeScenes.erase(activeIt);
}

void SceneManager::Update(float deltaTime)
{
    for (Scene* scene : m_activeScenes)
    {
        scene->Update(deltaTime);
    }
}

const std::vector<Scene*>& SceneManager::GetActiveScenes() const
{
    return m_activeScenes;
}
