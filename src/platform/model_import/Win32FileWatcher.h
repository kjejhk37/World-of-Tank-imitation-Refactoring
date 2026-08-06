#pragma once

#include <windows.h>

#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

#include "platform/model_import/IFileChangeDetector.h"

// Author: Claude
// Description: Win32 ReadDirectoryChangesW를 감싼 wrapper. 지정한 루트 디렉터리(하위 폴더 포함)를
//              전용 워커 스레드에서 비동기로 감시하고, 변경된 파일의 절대 경로를 기록한다.
// Input: 생성자 - 감시할 루트 디렉터리 경로
// Output: HasChanged - path가 마지막 확인 이후 변경되었으면 true(내부적으로 소비/클리어)
// Notes: Win32Window(src/platform)와 동일한 원칙으로 게임의 메인 윈도우 메시지 루프와는 완전히
//        독립적으로 동작한다 - 감시는 자체 스레드+이벤트 객체로만 이뤄진다.
//        ReadDirectoryChangesW가 돌려주는 파일명은 감시 루트 기준 상대 경로라, 절대 경로로
//        정규화(슬래시 통일 + 소문자화, Windows 파일시스템은 대소문자 구분 안 함)해 비교한다.
//        FILE_NOTIFY_INFORMATION의 파일명은 UTF-16이라 WideCharToMultiByte로 UTF-8 변환한다.
//        생성자는 워커 스레드가 첫 ReadDirectoryChangesW 호출을 실제로 걸 때까지 대기(m_readyEvent)한
//        뒤에 반환한다 - 그렇지 않으면 생성자 직후 곧바로 파일을 수정하는 호출자가 "아직 감시가
//        걸리기 전"에 변경을 일으켜 알림을 놓치는 경쟁 조건이 생긴다.
//        인스턴스 하나는 생성 시점에 고정된 단일 루트 디렉터리(하위 폴더 포함)만 감시한다 - 여러
//        디렉터리에 흩어진 파일을 핫리로드하려면 루트가 같은 상위 폴더를 골라야 하거나 워처를
//        여러 개 둬야 한다.
// Date: 2026-07-23
class Win32FileWatcher : public IFileChangeDetector
{
public:
    explicit Win32FileWatcher(const std::string& watchDirectory);
    ~Win32FileWatcher() override;

    Win32FileWatcher(const Win32FileWatcher&) = delete;
    Win32FileWatcher& operator=(const Win32FileWatcher&) = delete;

    bool HasChanged(const std::string& path) override;

private:
    void WatchLoop();

    HANDLE m_directoryHandle = INVALID_HANDLE_VALUE;
    HANDLE m_stopEvent = nullptr;
    HANDLE m_readyEvent = nullptr;
    std::string m_watchDirectoryNormalized;
    std::thread m_thread;

    std::mutex m_mutex;
    std::unordered_set<std::string> m_changedPaths;
};
