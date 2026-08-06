#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "platform/logging/ILogSink.h"
#include "platform/logging/LogLevel.h"

// Author: Claude
// Description: 로그 등급별 API(Info/Warning/Error)를 제공하고, 메시지를 포맷한 뒤 등록된 모든
//              ILogSink에 분배한다.
// Input: 생성자 - sinks (로그를 실제로 내보낼 대상 목록, 소유권은 Logger로 이전)
// Output: (해당 없음 — 부수효과로 각 싱크에 기록)
// Notes: Error(code, message)는 메시지 앞에 "[코드번호] "를 붙여 전달한다 — code는 platform이 의미를
//        모르는 순수 숫자다(FormatLogLine/ILogSink와 마찬가지로 어떤 에러 코드 카탈로그도 알지 못한다, SRP).
//        std::mutex로 분배 구간을 보호해 여러 스레드에서 동시에 호출해도 안전하다.
// Date: 2026-07-19
class Logger
{
public:
    explicit Logger(std::vector<std::unique_ptr<ILogSink>> sinks);

    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(int code, const std::string& message);

private:
    void Log(LogLevel level, const std::string& message);

    std::vector<std::unique_ptr<ILogSink>> m_sinks;
    std::mutex m_mutex;
};
