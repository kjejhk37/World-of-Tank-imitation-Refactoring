#pragma once

#include "logging/ILogSink.h"

// Author: Claude
// Description: 콘솔(cmd 창)이 연결되어 있을 때만 로그를 표준 출력/에러 스트림으로 내보내는 싱크.
// Input: (해당 없음 — 생성자 인자 없음)
// Output: (해당 없음 — 부수효과로 콘솔에 출력)
// Notes: 표준 출력/에러 핸들이 유효할 때만 기록한다(GetStdHandle로 확인) — 콘솔에 직접 붙어있든
//        파일/파이프로 리다이렉트되어 있든 둘 다 유효한 핸들이라 정상 기록되고, WIN32 서브시스템처럼
//        핸들 자체가 없는 경우에만 조용히 아무것도 하지 않는다.
//        Error/Warning은 std::cerr, Info는 std::cout으로 분리한다.
//        (구현 중 변경: 최초안은 GetConsoleWindow() != NULL을 검사했으나, 이 방식은 출력이 리다이렉트된
//         환경(ctest 캡처 등)에서 콘솔 창이 없다는 이유로 정상적인 리다이렉트 출력까지 막아버려 폐기함.)
// Date: 2026-07-19
class ConsoleLogSink : public ILogSink
{
public:
    void Write(LogLevel level, const std::string& formattedLine) override;
};
