#pragma once

#include <string>

#include "platform/logging/LogLevel.h"

// Author: Claude
// Description: 로그 한 줄을 실제 출력 대상(콘솔/파일 등)에 내보내는 책임을 감싸는 인터페이스.
//              Logger는 이 인터페이스만 알고, 구체적인 출력 매체를 모른다.
// Input: (해당 없음 — 인터페이스)
// Output: (해당 없음 — 인터페이스)
// Notes: formattedLine은 이미 FormatLogLine으로 조립된 완성된 문자열이다 — 싱크는 포맷을 다시 만들지 않는다(SRP).
// Date: 2026-07-19
class ILogSink
{
public:
    virtual ~ILogSink() = default;

    virtual void Write(LogLevel level, const std::string& formattedLine) = 0;
};
