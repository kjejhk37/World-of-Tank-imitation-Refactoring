#include "platform/logging/ConsoleLogSink.h"

#include <windows.h>

#include <iostream>

namespace
{
    bool HasValidStandardHandle(DWORD standardHandleId)
    {
        HANDLE handle = GetStdHandle(standardHandleId);
        return handle != NULL && handle != INVALID_HANDLE_VALUE;
    }
}

void ConsoleLogSink::Write(LogLevel level, const std::string& formattedLine)
{
    if (level == LogLevel::Info)
    {
        if (HasValidStandardHandle(STD_OUTPUT_HANDLE))
        {
            std::cout << formattedLine << std::endl;
        }
    }
    else if (HasValidStandardHandle(STD_ERROR_HANDLE))
    {
        std::cerr << formattedLine << std::endl;
    }
}
