#pragma once

#include <ctime>
#include <string>

#include "logging/LogLevel.h"

// Author: Claude
// Description: 로그 등급/시간/메시지를 "타입 : YYYY-MM-DD HH:MM:SS : 내용" 한 줄 문자열로 조립한다.
// Input: level - 로그 등급 / message - 로그 본문 / timestamp - 기록 시각(테스트에서 결정적 검증을 위해 주입 가능, 기본값은 현재 시각)
// Output: 조립된 한 줄 로그 문자열 (개행 문자는 포함하지 않음 — 싱크가 필요에 따라 붙인다)
// Notes: ErrorCode는 이 함수가 알지 못한다 — 코드 번호를 메시지에 넣는 책임은 Logger::Error에 있다(SRP).
// Date: 2026-07-19
std::string FormatLogLine(LogLevel level, const std::string& message, std::time_t timestamp = std::time(nullptr));
