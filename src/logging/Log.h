#pragma once

#include <string>

#include "logging/ErrorCode.h"

// Author: Claude
// Description: 프로세스 전역에서 접근 가능한 Logger 래퍼 (namespace 기반, GoF Singleton 클래스 아님).
//              LaunchConfigStore와 동일한 패턴으로, 코드베이스 어디서든 Log::Info/Warning/Error를 바로 호출할 수 있게 한다.
// Input: (Init) logFilePath (비어있으면 파일 출력 없이 콘솔 싱크만 사용) / (Info/Warning) message / (Error) code, message
// Output: (해당 없음 — 부수효과로 콘솔/파일에 기록)
// Notes: Init은 main() 진입 직후, 다른 스레드가 뜨기 전에 단 한 번만 호출해야 한다.
//        Init 이전에 Info/Warning/Error를 호출하면 미정의 동작이다(LaunchConfigStore와 동일한 계약).
// Date: 2026-07-19
namespace Log
{
    void Init(const std::string& logFilePath);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(ErrorCode code, const std::string& message);
}
