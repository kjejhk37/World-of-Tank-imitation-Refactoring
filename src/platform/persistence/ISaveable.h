#pragma once

#include "platform/serialization/DataRecord.h"

// Author: Claude
// Description: SaveLoadManager가 저장/복원할 수 있는 객체가 구현하는 인터페이스.
// Input: (해당 없음 - 인터페이스)
// Output: (해당 없음 - 인터페이스)
// Notes: SaveLoadManager는 이 인터페이스에만 의존하고 각 구현체의 내부 데이터 구조는 알지 못한다 (의존성 역전).
// Date: 2026-07-19
class ISaveable
{
public:
    virtual ~ISaveable() = default;

    virtual DataRecord ToRecord() const = 0;
    virtual void FromRecord(const DataRecord& record) = 0;
};
