#pragma once

#include <string>

#include "config/AppConfig.h"
#include "serialization/IDataStore.h"

// Author: Claude
// Description: IDataStore를 통해 JSON Config 파일을 읽어 AppConfig를 만든다. 파일이 없거나 필드가 누락되면 하드코딩 기본값을 사용한다.
// Input: 생성자 - Config 값을 읽어올 IDataStore / LoadOrDefault - Config 파일 경로로 쓰일 key
// Output: LoadOrDefault - 하드코딩 기본값 위에 파일 값을 덮어쓴 AppConfig
// Notes: 이 클래스는 IDataStore 구현체(JSON/DB)를 알지 못한다 - 생성자로 주입받은 인터페이스만 사용한다 (의존성 역전).
// Date: 2026-07-19
class ConfigManager
{
public:
    explicit ConfigManager(IDataStore& dataStore);

    AppConfig LoadOrDefault(const std::string& configFilePath) const;

private:
    IDataStore& m_dataStore;
};
