#pragma once

#include <string>

#include "persistence/ISaveable.h"
#include "serialization/IDataStore.h"

// Author: Claude
// Description: IDataStore를 통해 ISaveable 객체를 파일에 저장/복원한다. saveVersion 필드로 이후 스키마 변경에 대비한다.
// Input: 생성자 - 저장에 사용할 IDataStore / Save,Load - 대상 ISaveable, 저장 파일 경로로 쓰일 key
// Output: Save,Load - 성공 여부(bool)
// Notes: SaveAsync는 이번 사이클에서 시그니처만 제공하는 확장 지점이며 내부적으로 동기 Save를 그대로 호출한다.
//        실제 비동기(스레드) 구현은 별도 Task에서 진행한다.
// Date: 2026-07-19
class SaveLoadManager
{
public:
    explicit SaveLoadManager(IDataStore& dataStore);

    bool Save(const std::string& key, const ISaveable& target) const;
    bool Load(const std::string& key, ISaveable& target) const;

    // 비동기 저장 확장 지점 (스텁) - 실제 스레드 구현은 다음 Task.
    bool SaveAsync(const std::string& key, const ISaveable& target) const;

private:
    IDataStore& m_dataStore;
};
