#include "platform/serialization/DataRecordAccess.h"

#include <charconv>
#include <stdexcept>

namespace
{
    constexpr const char* kTrueValue = "true";
    constexpr const char* kFalseValue = "false";
}

namespace DataRecordAccess
{
    int GetIntOr(const DataRecord& record, const std::string& field, int fallback)
    {
        const auto it = record.find(field);
        if (it == record.end())
        {
            return fallback;
        }

        int value = fallback;
        const auto* begin = it->second.data();
        const auto* end = it->second.data() + it->second.size();
        const auto result = std::from_chars(begin, end, value);
        return result.ec == std::errc{} ? value : fallback;
    }

    bool GetBoolOr(const DataRecord& record, const std::string& field, bool fallback)
    {
        const auto it = record.find(field);
        if (it == record.end())
        {
            return fallback;
        }
        if (it->second == kTrueValue)
        {
            return true;
        }
        if (it->second == kFalseValue)
        {
            return false;
        }
        return fallback;
    }

    float GetFloatOr(const DataRecord& record, const std::string& field, float fallback)
    {
        const auto it = record.find(field);
        if (it == record.end())
        {
            return fallback;
        }

        try
        {
            return std::stof(it->second);
        }
        catch (const std::exception&)
        {
            return fallback;
        }
    }

    std::string GetStringOr(const DataRecord& record, const std::string& field, const std::string& fallback)
    {
        const auto it = record.find(field);
        return it == record.end() ? fallback : it->second;
    }
}
