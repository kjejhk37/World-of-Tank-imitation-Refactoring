#pragma once

#include <string>

#include "platform/serialization/DataRecord.h"

// Author: Claude
// Description: Config/Save-Load가 실제 저장 매체(JSON 파일, DB 등)를 몰라도 되게 감추는 저장소 인터페이스.
// Input: (해당 없음 - 인터페이스)
// Output: (해당 없음 - 인터페이스)
// Notes: 구현체를 교체(JSON -> DB)해도 이 인터페이스에만 의존하는 ConfigManager/SaveLoadManager는 수정이 필요 없다 (의존성 역전).
// Date: 2026-07-19
class IDataStore
{
public:
    virtual ~IDataStore() = default;

    virtual bool Save(const std::string& key, const DataRecord& record) = 0;
    virtual bool Load(const std::string& key, DataRecord& outRecord) = 0;
};
