#include "platform/Win32Window.h"

#include <stdexcept>

namespace
{
    constexpr const wchar_t* kWindowClassName = L"WorldOfTankImitationWindowClass";

    std::wstring ToWideString(const std::string& text)
    {
        if (text.empty())
        {
            return std::wstring();
        }

        const int requiredSize = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        std::wstring wide(static_cast<std::size_t>(requiredSize), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), requiredSize);
        wide.resize(wide.size() - 1);  // MultiByteToWideChar가 포함시킨 null 종단 문자 제거
        return wide;
    }
}

void Win32Window::RegisterWindowClass()
{
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpfnWndProc = &Win32Window::WndProc;
    windowClass.hInstance = GetModuleHandle(nullptr);
    windowClass.lpszClassName = kWindowClassName;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassExW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        throw std::runtime_error("Failed to register Win32 window class");
    }
}

Win32Window::Win32Window(int width, int height, const std::string& title, bool fullscreen)
{
    RegisterWindowClass();

    const std::wstring wideTitle = ToWideString(title);
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (fullscreen)
    {
        m_hwnd = CreateWindowExW(0, kWindowClassName, wideTitle.c_str(), WS_POPUP, 0, 0, screenWidth, screenHeight,
                                  nullptr, nullptr, GetModuleHandle(nullptr), this);
    }
    else
    {
        RECT rect{0, 0, width, height};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        const int outerWidth = rect.right - rect.left;
        const int outerHeight = rect.bottom - rect.top;
        const int x = (screenWidth - outerWidth) / 2;
        const int y = (screenHeight - outerHeight) / 2;

        m_hwnd = CreateWindowExW(0, kWindowClassName, wideTitle.c_str(), WS_OVERLAPPEDWINDOW, x, y, outerWidth,
                                  outerHeight, nullptr, nullptr, GetModuleHandle(nullptr), this);
    }

    if (m_hwnd == nullptr)
    {
        throw std::runtime_error("Failed to create Win32 window");
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
}

Win32Window::~Win32Window()
{
    if (m_hwnd != nullptr)
    {
        DestroyWindow(m_hwnd);
    }
}

HWND Win32Window::Handle() const
{
    return m_hwnd;
}

int Win32Window::ClientWidth() const
{
    RECT rect{};
    GetClientRect(m_hwnd, &rect);
    return rect.right - rect.left;
}

int Win32Window::ClientHeight() const
{
    RECT rect{};
    GetClientRect(m_hwnd, &rect);
    return rect.bottom - rect.top;
}

bool Win32Window::PumpMessages()
{
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Win32Window::SetResizeCallback(std::function<void(int, int)> callback)
{
    m_resizeCallback = std::move(callback);
}

void Win32Window::SetMessageHook(std::function<bool(HWND, UINT, WPARAM, LPARAM)> hook)
{
    m_messageHook = std::move(hook);
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Win32Window* window = nullptr;

    if (message == WM_NCCREATE)
    {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = reinterpret_cast<Win32Window*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
    {
        window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window != nullptr && window->m_messageHook && window->m_messageHook(hwnd, message, wParam, lParam))
    {
        return true;
    }

    switch (message)
    {
    case WM_SIZE:
    {
        const int width = LOWORD(lParam);
        const int height = HIWORD(lParam);
        if (window != nullptr && window->m_resizeCallback && width > 0 && height > 0)
        {
            window->m_resizeCallback(width, height);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}
