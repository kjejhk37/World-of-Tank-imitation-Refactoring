#include "model_import/Win32FileWatcher.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <vector>

namespace
{

std::string NormalizePath(const std::string& path)
{
    char fullPath[MAX_PATH] = {};
    const DWORD length = GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr);

    std::string result = (length > 0 && length < MAX_PATH) ? std::string(fullPath, length) : path;
    for (char& c : result)
    {
        if (c == '/')
        {
            c = '\\';
        }
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string WideToUtf8(const wchar_t* data, size_t lengthInChars)
{
    if (lengthInChars == 0)
    {
        return std::string();
    }
    const int required = WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(lengthInChars), nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<size_t>(required), '\0');
    WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(lengthInChars), result.data(), required, nullptr, nullptr);
    return result;
}

}  // namespace

Win32FileWatcher::Win32FileWatcher(const std::string& watchDirectory)
{
    m_watchDirectoryNormalized = NormalizePath(watchDirectory);

    m_directoryHandle = CreateFileA(watchDirectory.c_str(), FILE_LIST_DIRECTORY,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                                     FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (m_directoryHandle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Win32FileWatcher: failed to open directory '" + watchDirectory + "'");
    }

    m_stopEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    m_readyEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    m_thread = std::thread(&Win32FileWatcher::WatchLoop, this);

    // 워커 스레드가 첫 ReadDirectoryChangesW를 실제로 걸 때까지 대기한다 - 그 전에 반환하면
    // 호출자가 곧바로 파일을 수정할 때 감시가 아직 걸리기 전이라 알림을 놓칠 수 있다.
    WaitForSingleObject(m_readyEvent, INFINITE);
}

Win32FileWatcher::~Win32FileWatcher()
{
    if (m_stopEvent != nullptr)
    {
        SetEvent(m_stopEvent);
    }
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    if (m_directoryHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_directoryHandle);
    }
    if (m_stopEvent != nullptr)
    {
        CloseHandle(m_stopEvent);
    }
    if (m_readyEvent != nullptr)
    {
        CloseHandle(m_readyEvent);
    }
}

bool Win32FileWatcher::HasChanged(const std::string& path)
{
    const std::string normalized = NormalizePath(path);

    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_changedPaths.find(normalized);
    if (it == m_changedPaths.end())
    {
        return false;
    }
    m_changedPaths.erase(it);
    return true;
}

void Win32FileWatcher::WatchLoop()
{
    std::vector<BYTE> buffer(64 * 1024);

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);

    while (true)
    {
        DWORD bytesReturned = 0;
        const BOOL requestOk =
            ReadDirectoryChangesW(m_directoryHandle, buffer.data(), static_cast<DWORD>(buffer.size()), TRUE,
                                   FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME, &bytesReturned,
                                   &overlapped, nullptr);
        if (!requestOk && GetLastError() != ERROR_IO_PENDING)
        {
            SetEvent(m_readyEvent);
            break;
        }

        // ReadDirectoryChangesW 호출이 (동기 완료든 비동기 대기 중이든) 실제로 걸렸으므로,
        // 생성자에서 대기 중인 호출자를 이제 깨워도 안전하다.
        SetEvent(m_readyEvent);

        HANDLE waitHandles[2] = {overlapped.hEvent, m_stopEvent};
        const DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0 + 1)
        {
            CancelIoEx(m_directoryHandle, &overlapped);
            break;
        }

        DWORD transferred = 0;
        if (!GetOverlappedResult(m_directoryHandle, &overlapped, &transferred, FALSE) || transferred == 0)
        {
            ResetEvent(overlapped.hEvent);
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            size_t offset = 0;
            while (offset < transferred)
            {
                const auto* info = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(buffer.data() + offset);
                const std::string relativeName =
                    WideToUtf8(info->FileName, info->FileNameLength / sizeof(WCHAR));
                m_changedPaths.insert(NormalizePath(m_watchDirectoryNormalized + "\\" + relativeName));

                if (info->NextEntryOffset == 0)
                {
                    break;
                }
                offset += info->NextEntryOffset;
            }
        }

        ResetEvent(overlapped.hEvent);
    }

    CloseHandle(overlapped.hEvent);
}
