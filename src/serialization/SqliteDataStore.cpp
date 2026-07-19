#include "serialization/SqliteDataStore.h"

#include <stdexcept>

bool SqliteDataStore::Save(const std::string& /*key*/, const DataRecord& /*record*/)
{
    throw std::logic_error("SqliteDataStore::Save is not implemented yet");
}

bool SqliteDataStore::Load(const std::string& /*key*/, DataRecord& /*outRecord*/)
{
    throw std::logic_error("SqliteDataStore::Load is not implemented yet");
}
