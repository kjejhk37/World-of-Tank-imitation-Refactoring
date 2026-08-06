#pragma once

#include <string>
#include <vector>

#include "platform/persistence/ISaveable.h"

// Author: Claude
// Description: Save/Load 시스템 동작 검증에 쓰는 최소 예시 데이터 - 재화와 해금한 탱크 ID 목록.
// Input: (해당 없음 - 데이터 구조체 겸 ISaveable 구현체)
// Output: (해당 없음)
// Notes: 실제 게임 데이터 모델과의 연동은 이번 사이클 범위 밖이며, SaveLoadManager 동작을 검증하기 위한 예시일 뿐이다.
// Date: 2026-07-19
class PlayerProgress : public ISaveable
{
public:
    int currency = 0;
    std::vector<std::string> unlockedTankIds;

    DataRecord ToRecord() const override;
    void FromRecord(const DataRecord& record) override;
};
