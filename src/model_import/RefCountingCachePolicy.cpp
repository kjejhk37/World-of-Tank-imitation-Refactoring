#include "model_import/RefCountingCachePolicy.h"

std::shared_ptr<const Model> RefCountingCachePolicy::TryGet(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_entries.find(key);
    if (it == m_entries.end())
    {
        return nullptr;
    }
    return it->second.lock();
}

void RefCountingCachePolicy::Store(const std::string& key, std::shared_ptr<const Model> model)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries[key] = std::move(model);
}

void RefCountingCachePolicy::Unload(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.erase(key);
}
