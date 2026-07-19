#pragma once

#include <fstream>
#include <string>

#include "logging/ILogSink.h"

// Author: Claude
// Description: 로그를 지정된 .txt 파일에 실시간(매 호출 후 flush)으로 남기는 싱크.
// Input: 생성자 - filePath (파일 경로, 비어있으면 파일을 열지 않고 이후 Write는 전부 no-op)
// Output: (해당 없음 — 부수효과로 파일에 기록) / IsEnabled() - 실제로 파일이 열려 기록 중인지 여부(테스트 검증용)
// Notes: 널 오브젝트 패턴 — 경로 미지정/파일 열기 실패 시 Logger 쪽에 별도 분기를 두지 않고 이 싱크가 조용히 아무것도 안 한다.
//        append 모드로 한 번 열어 유지하고, 매 Write 호출 직후 flush() — 원인 모를 크래시 직전까지의 로그를 보존하기 위함.
// Date: 2026-07-19
class FileLogSink : public ILogSink
{
public:
    explicit FileLogSink(const std::string& filePath);

    void Write(LogLevel level, const std::string& formattedLine) override;
    bool IsEnabled() const;

private:
    std::ofstream m_file;
};
