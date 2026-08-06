#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "platform/logging/FileLogSink.h"

namespace
{
    std::filesystem::path MakeTempFilePath(const char* fileName)
    {
        return std::filesystem::temp_directory_path() / fileName;
    }

    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}

TEST(FileLogSinkTest, WriteAppendsAndFlushesImmediately)
{
    const std::filesystem::path path = MakeTempFilePath("FileLogSinkTest_WriteAppendsAndFlushesImmediately.txt");
    std::filesystem::remove(path);

    {
        FileLogSink sink(path.string());
        EXPECT_TRUE(sink.IsEnabled());

        sink.Write(LogLevel::Info, "Info : 2026-07-19 15:08:00 : first");
        EXPECT_NE(ReadFile(path).find("first"), std::string::npos);

        sink.Write(LogLevel::Error, "Error : 2026-07-19 15:08:01 : second");
        const std::string content = ReadFile(path);
        EXPECT_NE(content.find("first"), std::string::npos);
        EXPECT_NE(content.find("second"), std::string::npos);
    }

    std::filesystem::remove(path);
}

TEST(FileLogSinkTest, EmptyPathNeverCreatesFile)
{
    FileLogSink sink("");
    EXPECT_FALSE(sink.IsEnabled());

    sink.Write(LogLevel::Error, "Error : 2026-07-19 15:08:00 : should not persist");
    EXPECT_FALSE(sink.IsEnabled());
}
