#include "platform/logging/LogFormatter.h"

#include <iomanip>
#include <sstream>

namespace
{
    const char* LogLevelToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
        default:
            return "Unknown";
        }
    }
}

std::string FormatLogLine(LogLevel level, const std::string& message, std::time_t timestamp)
{
    std::tm localTime{};
    localtime_s(&localTime, &timestamp);

    std::ostringstream stream;
    stream << LogLevelToString(level) << " : " << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << " : " << message;
    return stream.str();
}
