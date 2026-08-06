#pragma once

#include "platform/serialization/IDataStore.h"

// Author: Claude
// Description: IDataStore를 JSON 파일 기반으로 구현한다. nlohmann::json에 대한 직접 참조는 이 클래스 내부(.cpp)로 한정한다.
// Input: (Save) 파일 경로로 쓸 key, 저장할 DataRecord / (Load) 읽어올 key
// Output: (Save/Load) 성공 여부(bool). Load 성공 시 outRecord에 값을 채운다.
// Notes: key는 파일 경로로 취급한다. 파일이 없거나 최상위가 JSON object가 아니면 Load는 예외 없이 false를 반환한다.
// Date: 2026-07-19
class JsonDataStore : public IDataStore
{
public:
    bool Save(const std::string& key, const DataRecord& record) override;
    bool Load(const std::string& key, DataRecord& outRecord) override;
};
