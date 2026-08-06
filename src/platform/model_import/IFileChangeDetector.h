#pragma once

#include <string>

// Author: Claude
// Description: HotReloadCachePolicy가 "이 파일이 마지막 확인 이후 바뀌었는가"를 물어보는
//              감지 메커니즘 인터페이스.
// Input: (해당 없음 - 인터페이스)
// Output: (해당 없음 - 인터페이스)
// Notes: 이번 사이클은 Win32FileWatcher(ReadDirectoryChangesW 기반, 이벤트 방식)만 구현한다.
//        PollingHashFileWatcher(수정 시각/해시 비교, Unity의 Refresh 방식)는 이 인터페이스만
//        열어두고 백로그로 미룬다(OCP - 나중에 클래스 하나만 추가하면 됨).
// Date: 2026-07-23
class IFileChangeDetector
{
public:
    virtual ~IFileChangeDetector() = default;

    // path에 해당하는 파일이 마지막 HasChanged 호출 이후 변경되었으면 true를 반환하고
    // 내부적으로 "변경됨" 상태를 소비(클리어)한다.
    virtual bool HasChanged(const std::string& path) = 0;
};
