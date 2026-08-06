#include <gtest/gtest.h>
#include <windows.h>

#include <chrono>
#include <fstream>
#include <thread>

#include "platform/model_import/Win32FileWatcher.h"

namespace
{

// 비동기 감시 스레드가 알림을 처리할 시간을 벌어주기 위해, 고정 sleep 대신 상한 시간까지 폴링한다
// (Win32WindowTest의 순서 의존 플레이키니스 해소 경험과 동일하게, 고정 지연 대신 조건 폴링을 쓴다).
bool WaitUntilChanged(IFileChangeDetector& detector, const std::string& path,
                      std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (detector.HasChanged(path))
        {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return false;
}

std::string MakeTempDirectory()
{
    char tempPath[MAX_PATH] = {};
    GetTempPathA(MAX_PATH, tempPath);

    char uniqueDir[MAX_PATH] = {};
    GetTempFileNameA(tempPath, "mdl", 0, uniqueDir);

    DeleteFileA(uniqueDir);
    CreateDirectoryA(uniqueDir, nullptr);
    return std::string(uniqueDir);
}

}  // namespace

TEST(Win32FileWatcherTest, DetectsFileModificationWithinWatchedDirectory)
{
    const std::string dir = MakeTempDirectory();
    const std::string filePath = dir + "\\watched.txt";

    {
        std::ofstream initial(filePath);
        initial << "initial";
    }

    Win32FileWatcher watcher(dir);

    {
        std::ofstream modify(filePath, std::ios::trunc);
        modify << "modified";
    }

    EXPECT_TRUE(WaitUntilChanged(watcher, filePath, std::chrono::milliseconds(2000)));

    DeleteFileA(filePath.c_str());
    RemoveDirectoryA(dir.c_str());
}

TEST(Win32FileWatcherTest, HasChangedConsumesTheChangeFlag)
{
    const std::string dir = MakeTempDirectory();
    const std::string filePath = dir + "\\watched.txt";

    {
        std::ofstream initial(filePath);
        initial << "initial";
    }

    Win32FileWatcher watcher(dir);

    {
        std::ofstream modify(filePath, std::ios::trunc);
        modify << "modified";
    }

    ASSERT_TRUE(WaitUntilChanged(watcher, filePath, std::chrono::milliseconds(2000)));
    // 위 WaitUntilChanged가 이미 변경 플래그를 소비했으므로, 추가 수정이 없으면 다시 물어봐도 false다.
    EXPECT_FALSE(watcher.HasChanged(filePath));

    DeleteFileA(filePath.c_str());
    RemoveDirectoryA(dir.c_str());
}
