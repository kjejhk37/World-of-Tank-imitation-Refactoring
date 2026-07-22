#include <gtest/gtest.h>

#include "platform/Win32Window.h"

namespace
{
    // Win32Window 소멸자는 WM_DESTROY 시 PostQuitMessage(0)을 호출한다(프로덕션의 단일 윈도우
    // 앱에서는 의도된 동작). 같은 스레드에서 윈도우를 연속 생성/파괴하는 테스트에서는 이전 테스트가
    // 남긴 WM_QUIT이 다음 테스트의 메시지 큐에 남아 PumpMessages()를 오염시키므로, 각 테스트 시작
    // 전 잔여 메시지를 비워 테스트 간 순서 의존성을 없앤다.
    void DrainPendingMessages()
    {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
        }
    }
}

class Win32WindowTest : public ::testing::Test
{
protected:
    void SetUp() override { DrainPendingMessages(); }
};

TEST_F(Win32WindowTest, CreatesValidWindowedHandle)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);
    EXPECT_NE(window.Handle(), nullptr);
}

TEST_F(Win32WindowTest, CreatesValidFullscreenHandle)
{
    Win32Window window(1280, 720, "Win32WindowTest", true);
    EXPECT_NE(window.Handle(), nullptr);
}

TEST_F(Win32WindowTest, FullscreenClientSizeMatchesScreenResolution)
{
    Win32Window window(1280, 720, "Win32WindowTest", true);

    EXPECT_EQ(window.ClientWidth(), GetSystemMetrics(SM_CXSCREEN));
    EXPECT_EQ(window.ClientHeight(), GetSystemMetrics(SM_CYSCREEN));
}

TEST_F(Win32WindowTest, PumpMessagesReturnsTrueWhenNoQuitPosted)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);
    EXPECT_TRUE(window.PumpMessages());
}

TEST_F(Win32WindowTest, MessageHookReceivesMessagesAndCanClaimThem)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);

    bool hookCalled = false;
    UINT receivedMessage = 0;
    window.SetMessageHook([&hookCalled, &receivedMessage](HWND, UINT message, WPARAM, LPARAM) {
        if (message == WM_APP)
        {
            hookCalled = true;
            receivedMessage = message;
            return true;  // 소비했다고 응답 — WndProc이 그 자리에서 반환해야 함
        }
        return false;
    });

    const LRESULT result = SendMessage(window.Handle(), WM_APP, 0, 0);

    EXPECT_TRUE(hookCalled);
    EXPECT_EQ(receivedMessage, static_cast<UINT>(WM_APP));
    EXPECT_EQ(result, TRUE);
}

TEST_F(Win32WindowTest, MessageHookReturningFalseDoesNotBlockResizeCallback)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);

    window.SetMessageHook([](HWND, UINT, WPARAM, LPARAM) { return false; });

    bool resizeCallbackCalled = false;
    window.SetResizeCallback([&resizeCallbackCalled](int, int) { resizeCallbackCalled = true; });

    SetWindowPos(window.Handle(), nullptr, 0, 0, 800, 600, SWP_NOMOVE | SWP_NOZORDER);

    EXPECT_TRUE(resizeCallbackCalled);
}
