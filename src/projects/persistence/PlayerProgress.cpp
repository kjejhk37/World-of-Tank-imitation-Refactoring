#include "projects/persistence/PlayerProgress.h"

#include <sstream>

#include "platform/serialization/DataRecordAccess.h"

// 로컬 상수 분리 기준: 이 파일이 500줄을 넘거나 아래 상수가 5개를 넘으면
// 별도 헤더로 분리한다.
namespace
{
    constexpr const char* kCurrencyField = "currency";
    constexpr const char* kUnlockedTankIdsField = "unlockedTankIds";
    constexpr char kListDelimiter = ',';

    std::string Join(const std::vector<std::string>& values)
    {
        std::string joined;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                joined += kListDelimiter;
            }
            joined += values[i];
        }
        return joined;
    }

    std::vector<std::string> Split(const std::string& text)
    {
        std::vector<std::string> values;
        if (text.empty())
        {
            return values;
        }

        std::stringstream stream(text);
        std::string token;
        while (std::getline(stream, token, kListDelimiter))
        {
            values.push_back(token);
        }
        return values;
    }
}

DataRecord PlayerProgress::ToRecord() const
{
    DataRecord record;
    record[kCurrencyField] = std::to_string(currency);
    record[kUnlockedTankIdsField] = Join(unlockedTankIds);
    return record;
}

void PlayerProgress::FromRecord(const DataRecord& record)
{
    currency = DataRecordAccess::GetIntOr(record, kCurrencyField, currency);
    if (const auto it = record.find(kUnlockedTankIdsField); it != record.end())
    {
        unlockedTankIds = Split(it->second);
    }
}
