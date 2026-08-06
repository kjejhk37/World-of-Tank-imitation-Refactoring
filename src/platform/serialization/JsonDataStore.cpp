#include "platform/serialization/JsonDataStore.h"

#include <fstream>

#include <nlohmann/json.hpp>

bool JsonDataStore::Save(const std::string& key, const DataRecord& record)
{
    nlohmann::json json;
    for (const auto& [field, value] : record)
    {
        json[field] = value;
    }

    std::ofstream file(key);
    if (!file.is_open())
    {
        return false;
    }

    file << json.dump(4);
    return true;
}

bool JsonDataStore::Load(const std::string& key, DataRecord& outRecord)
{
    std::ifstream file(key);
    if (!file.is_open())
    {
        return false;
    }

    nlohmann::json json;
    try
    {
        file >> json;
    }
    catch (const nlohmann::json::parse_error&)
    {
        return false;
    }

    if (!json.is_object())
    {
        return false;
    }

    DataRecord record;
    for (auto it = json.begin(); it != json.end(); ++it)
    {
        record[it.key()] = it->is_string() ? it->get<std::string>() : it->dump();
    }

    outRecord = std::move(record);
    return true;
}
