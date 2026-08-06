#include "platform/persistence/SaveLoadManager.h"

// 로컬 상수 분리 기준: 이 파일이 500줄을 넘거나 아래 상수가 5개를 넘으면
// 별도 헤더로 분리한다.
namespace
{
    constexpr const char* kSaveVersionField = "saveVersion";
    constexpr const char* kCurrentSaveVersion = "1";
}

SaveLoadManager::SaveLoadManager(IDataStore& dataStore)
    : m_dataStore(dataStore)
{
}

bool SaveLoadManager::Save(const std::string& key, const ISaveable& target) const
{
    DataRecord record = target.ToRecord();
    record[kSaveVersionField] = kCurrentSaveVersion;
    return m_dataStore.Save(key, record);
}

bool SaveLoadManager::Load(const std::string& key, ISaveable& target) const
{
    DataRecord record;
    if (!m_dataStore.Load(key, record))
    {
        return false;
    }

    target.FromRecord(record);
    return true;
}

bool SaveLoadManager::SaveAsync(const std::string& key, const ISaveable& target) const
{
    // 이번 사이클 범위: 동기 Save로 위임. 실제 백그라운드 스레드 처리는 다음 Task에서 구현.
    return Save(key, target);
}
