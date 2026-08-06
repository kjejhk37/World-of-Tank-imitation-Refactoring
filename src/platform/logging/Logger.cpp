#include "platform/logging/Logger.h"

#include "platform/logging/LogFormatter.h"

Logger::Logger(std::vector<std::unique_ptr<ILogSink>> sinks)
    : m_sinks(std::move(sinks))
{
}

void Logger::Info(const std::string& message)
{
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::string& message)
{
    Log(LogLevel::Warning, message);
}

void Logger::Error(int code, const std::string& message)
{
    Log(LogLevel::Error, "[" + std::to_string(code) + "] " + message);
}

void Logger::Log(LogLevel level, const std::string& message)
{
    const std::string line = FormatLogLine(level, message);

    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& sink : m_sinks)
    {
        sink->Write(level, line);
    }
}
