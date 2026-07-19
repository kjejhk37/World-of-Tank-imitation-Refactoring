#include <gtest/gtest.h>

#include "logging/ConsoleLogSink.h"

TEST(ConsoleLogSinkTest, InfoWritesToStdoutOnly)
{
    ConsoleLogSink sink;

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    sink.Write(LogLevel::Info, "Info : 2026-07-19 15:08:00 : hello");
    const std::string capturedStdout = testing::internal::GetCapturedStdout();
    const std::string capturedStderr = testing::internal::GetCapturedStderr();

    EXPECT_NE(capturedStdout.find("hello"), std::string::npos);
    EXPECT_TRUE(capturedStderr.empty());
}

TEST(ConsoleLogSinkTest, WarningWritesToStderrOnly)
{
    ConsoleLogSink sink;

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    sink.Write(LogLevel::Warning, "Warning : 2026-07-19 15:08:00 : careful");
    const std::string capturedStdout = testing::internal::GetCapturedStdout();
    const std::string capturedStderr = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(capturedStdout.empty());
    EXPECT_NE(capturedStderr.find("careful"), std::string::npos);
}

TEST(ConsoleLogSinkTest, ErrorWritesToStderrOnly)
{
    ConsoleLogSink sink;

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    sink.Write(LogLevel::Error, "Error : 2026-07-19 15:08:00 : broken");
    const std::string capturedStdout = testing::internal::GetCapturedStdout();
    const std::string capturedStderr = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(capturedStdout.empty());
    EXPECT_NE(capturedStderr.find("broken"), std::string::npos);
}
