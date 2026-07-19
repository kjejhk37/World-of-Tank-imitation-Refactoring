#include "logging/Log.h"

#include <memory>
#include <vector>

#include "logging/ConsoleLogSink.h"
#include "logging/FileLogSink.h"
#include "logging/Logger.h"

namespace
{
    std::unique_ptr<Logger> g_logger;
}

namespace Log
{
    void Init(const std::string& logFilePath)
    {
        std::vector<std::unique_ptr<ILogSink>> sinks;
        sinks.push_back(std::make_unique<ConsoleLogSink>());
        sinks.push_back(std::make_unique<FileLogSink>(logFilePath));
        g_logger = std::make_unique<Logger>(std::move(sinks));
    }

    void Info(const std::string& message)
    {
        g_logger->Info(message);
    }

    void Warning(const std::string& message)
    {
        g_logger->Warning(message);
    }

    void Error(ErrorCode code, const std::string& message)
    {
        g_logger->Error(code, message);
    }
}
