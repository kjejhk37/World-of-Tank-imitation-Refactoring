#include "platform/model_import/LruCachePolicy.h"

LruCachePolicy::LruCachePolicy(size_t maxEntries) : m_maxEntries(maxEntries)
{
}

std::shared_ptr<const Model> LruCachePolicy::TryGet(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto it = m_registry.find(key);
    if (it == m_registry.end())
    {
        return nullptr;
    }

    std::shared_ptr<const Model> model = it->second.lock();
    if (model == nullptr)
    {
        m_registry.erase(it);
        return nullptr;
    }

    Touch(key, model);
    return model;
}

void LruCachePolicy::Store(const std::string& key, std::shared_ptr<const Model> model)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_registry[key] = model;
    Touch(key, model);
}

void LruCachePolicy::Unload(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_registry.erase(key);

    const auto it = m_recencyIndex.find(key);
    if (it != m_recencyIndex.end())
    {
        m_recency.erase(it->second);
        m_recencyIndex.erase(it);
    }
}

void LruCachePolicy::Touch(const std::string& key, const std::shared_ptr<const Model>& model)
{
    const auto existing = m_recencyIndex.find(key);
    if (existing != m_recencyIndex.end())
    {
        m_recency.erase(existing->second);
        m_recencyIndex.erase(existing);
    }

    m_recency.emplace_front(key, model);
    m_recencyIndex[key] = m_recency.begin();

    while (m_recency.size() > m_maxEntries)
    {
        const std::string& evictedKey = m_recency.back().first;
        m_recencyIndex.erase(evictedKey);
        m_recency.pop_back();
    }
}
