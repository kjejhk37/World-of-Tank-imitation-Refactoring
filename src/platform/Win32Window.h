#pragma once

#include <windows.h>

#include <functional>
#include <string>

// Author: Claude
// Description: Win32 윈도우 생성과 메시지 펌프를 감싸는 wrapper. IRenderer를 전혀 알지 못하며,
//              어떤 렌더러 백엔드(DirectX9/11/12, OpenGL)를 쓰든 재사용 가능하다.
// Input: 생성자 - 클라이언트 영역 너비/높이, 제목, 전체화면 여부
// Output: Handle() - 생성된 HWND / ClientWidth(),ClientHeight() - 실제 클라이언트 영역 크기(전체화면이면 모니터 해상도라 생성자 인자와 다를 수 있음)
//         / PumpMessages() - 계속 실행해야 하면 true, WM_QUIT 수신 시 false
// Notes: WS_OVERLAPPEDWINDOW 스타일(창 모드)로 생성해 테두리 드래그로 크기 조절이 가능하다.
//        전체화면은 테두리 없는 전체화면(borderless, WS_POPUP + 모니터 해상도)으로 구현한다.
//        리사이즈 발생 시 렌더러에게 알리는 방법은 콜백(SetResizeCallback)으로 분리되어 있어 IRenderer에 의존하지 않는다(SRP).
//        SetMessageHook은 같은 이유로 존재하는 범용 콜백이다 — ImGui 같은 UI 프레임워크가 원시 Win32
//        메시지(마우스/키보드 등)를 가로채야 할 때 쓰지만, Win32Window는 그 콜백이 무엇을 하는지 전혀
//        모른다. 훅이 true(처리됨)를 반환하면 WndProc은 그 자리에서 반환하고 기존 처리(WM_SIZE 등)를
//        건너뛴다 — ImGui의 Win32 핸들러는 WM_SIZE/WM_DESTROY에는 절대 true를 반환하지 않으므로
//        기존 리사이즈/종료 처리와 충돌하지 않는다.
// Date: 2026-07-19
class Win32Window
{
public:
    Win32Window(int width, int height, const std::string& title, bool fullscreen);
    ~Win32Window();

    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    HWND Handle() const;
    int ClientWidth() const;
    int ClientHeight() const;
    bool PumpMessages();
    void SetResizeCallback(std::function<void(int, int)> callback);
    void SetMessageHook(std::function<bool(HWND, UINT, WPARAM, LPARAM)> hook);

private:
    HWND m_hwnd = nullptr;
    std::function<void(int, int)> m_resizeCallback;
    std::function<bool(HWND, UINT, WPARAM, LPARAM)> m_messageHook;

    static void RegisterWindowClass();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
