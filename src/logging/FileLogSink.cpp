#include "logging/FileLogSink.h"

FileLogSink::FileLogSink(const std::string& filePath)
{
    if (filePath.empty())
    {
        return;
    }

    m_file.open(filePath, std::ios::app);
}

void FileLogSink::Write(LogLevel /*level*/, const std::string& formattedLine)
{
    if (!m_file.is_open())
    {
        return;
    }

    m_file << formattedLine << "\n";
    m_file.flush();
}

bool FileLogSink::IsEnabled() const
{
    return m_file.is_open();
}
