#pragma once

#include "serialization/IDataStore.h"

// Author: Claude
// Description: IDataStore의 SQLite 기반 구현 스텁. 향후 저장 데이터가 커져 DB로 옮길 때 이 클래스만 실제로 구현하면 되도록 자리를 마련해 둔다.
// Input: (Save) key, DataRecord / (Load) key
// Output: (Save/Load) bool (단, 현재는 항상 예외를 던진다)
// Notes: 이번 사이클에서는 SQLite 라이브러리를 링크하지 않으며, 두 메서드 모두 호출 시 std::logic_error를 던진다.
//        실제 구현 시 SQLite C API/래퍼 라이브러리에 대한 별도 dependency_eval이 필요하다.
// Date: 2026-07-19
class SqliteDataStore : public IDataStore
{
public:
    bool Save(const std::string& key, const DataRecord& record) override;
    bool Load(const std::string& key, DataRecord& outRecord) override;
};
