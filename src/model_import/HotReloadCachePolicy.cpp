#include "model_import/HotReloadCachePolicy.h"

HotReloadCachePolicy::HotReloadCachePolicy(std::unique_ptr<IFileChangeDetector> detector)
    : m_detector(std::move(detector))
{
}

std::shared_ptr<const Model> HotReloadCachePolicy::TryGet(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_detector->HasChanged(key))
    {
        m_entries.erase(key);
        return nullptr;
    }

    const auto it = m_entries.find(key);
    return it != m_entries.end() ? it->second : nullptr;
}

void HotReloadCachePolicy::Store(const std::string& key, std::shared_ptr<const Model> model)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries[key] = std::move(model);
}

void HotReloadCachePolicy::Unload(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.erase(key);
}
