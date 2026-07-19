#include "config/ConfigManager.h"

#include "config/ConfigFields.h"
#include "renderer/RendererBackendParser.h"
#include "serialization/DataRecordAccess.h"

namespace
{
    RendererBackend ParseRendererBackendOr(const DataRecord& record, const std::string& field, RendererBackend fallback)
    {
        const std::string value = DataRecordAccess::GetStringOr(record, field, "");
        return RendererBackendParser::TryParse(value).value_or(fallback);
    }
}

ConfigManager::ConfigManager(IDataStore& dataStore)
    : m_dataStore(dataStore)
{
}

AppConfig ConfigManager::LoadOrDefault(const std::string& configFilePath) const
{
    AppConfig config;

    DataRecord record;
    if (!m_dataStore.Load(configFilePath, record))
    {
        return config;
    }

    config.width = DataRecordAccess::GetIntOr(record, ConfigFields::kWidth, config.width);
    config.height = DataRecordAccess::GetIntOr(record, ConfigFields::kHeight, config.height);
    config.fullscreen = DataRecordAccess::GetBoolOr(record, ConfigFields::kFullscreen, config.fullscreen);
    config.volume = DataRecordAccess::GetFloatOr(record, ConfigFields::kVolume, config.volume);
    config.renderer = ParseRendererBackendOr(record, ConfigFields::kRenderer, config.renderer);

    return config;
}
