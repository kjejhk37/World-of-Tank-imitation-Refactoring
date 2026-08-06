#include <gtest/gtest.h>

#include <ctime>

#include "platform/logging/LogFormatter.h"

namespace
{
    // mktime/localtime_s는 로컬 타임존을 기준으로 왕복하므로, 이 값으로 만든 time_t를
    // FormatLogLine에 다시 넣으면 실행 머신의 타임존과 무관하게 같은 tm 필드로 되돌아온다.
    std::time_t MakeTimestamp(int year, int month, int day, int hour, int minute, int second)
    {
        std::tm tm{};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1;
        return std::mktime(&tm);
    }
}

TEST(LogFormatterTest, FormatsInfoLevel)
{
    const std::time_t timestamp = MakeTimestamp(2026, 7, 19, 15, 8, 0);
    EXPECT_EQ(FormatLogLine(LogLevel::Info, "message", timestamp), "Info : 2026-07-19 15:08:00 : message");
}

TEST(LogFormatterTest, FormatsWarningLevel)
{
    const std::time_t timestamp = MakeTimestamp(2026, 7, 19, 15, 8, 0);
    EXPECT_EQ(FormatLogLine(LogLevel::Warning, "message", timestamp), "Warning : 2026-07-19 15:08:00 : message");
}

TEST(LogFormatterTest, FormatsErrorLevel)
{
    const std::time_t timestamp = MakeTimestamp(2026, 7, 19, 15, 8, 0);
    EXPECT_EQ(FormatLogLine(LogLevel::Error, "message", timestamp), "Error : 2026-07-19 15:08:00 : message");
}
