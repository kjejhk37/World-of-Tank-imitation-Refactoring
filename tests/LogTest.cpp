#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "platform/logging/Log.h"

namespace
{
    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}

TEST(LogTest, WithoutFilePathOnlyLogsToConsole)
{
    Log::Init("");

    testing::internal::CaptureStdout();
    Log::Info("console only");
    const std::string capturedStdout = testing::internal::GetCapturedStdout();

    EXPECT_NE(capturedStdout.find("console only"), std::string::npos);
}

TEST(LogTest, WithFilePathWritesToFileImmediately)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "LogTest_WithFilePathWritesToFileImmediately.txt";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    Log::Init(path.string());
    Log::Error(3002, "renderer init failed");

    const std::string content = ReadFile(path);
    EXPECT_NE(content.find("[3002]"), std::string::npos);
    EXPECT_NE(content.find("renderer init failed"), std::string::npos);

    // Log::Init이 구성한 FileLogSink는 프로세스 전역(g_logger)이 살아있는 동안 파일 핸들을 계속 들고
    // 있으므로 이 시점에 삭제할 수 없다(Windows는 열린 핸들이 있는 파일 삭제를 거부) — 삭제는 시도만 하고
    // 실패해도 무시한다(non-throwing overload).
    std::filesystem::remove(path, ec);
}
