#include <gtest/gtest.h>

#include "platform/Win32Window.h"

TEST(Win32WindowTest, CreatesValidWindowedHandle)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);
    EXPECT_NE(window.Handle(), nullptr);
}

TEST(Win32WindowTest, CreatesValidFullscreenHandle)
{
    Win32Window window(1280, 720, "Win32WindowTest", true);
    EXPECT_NE(window.Handle(), nullptr);
}

TEST(Win32WindowTest, FullscreenClientSizeMatchesScreenResolution)
{
    Win32Window window(1280, 720, "Win32WindowTest", true);

    EXPECT_EQ(window.ClientWidth(), GetSystemMetrics(SM_CXSCREEN));
    EXPECT_EQ(window.ClientHeight(), GetSystemMetrics(SM_CYSCREEN));
}

TEST(Win32WindowTest, PumpMessagesReturnsTrueWhenNoQuitPosted)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);
    EXPECT_TRUE(window.PumpMessages());
}

TEST(Win32WindowTest, MessageHookReceivesMessagesAndCanClaimThem)
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

TEST(Win32WindowTest, MessageHookReturningFalseDoesNotBlockResizeCallback)
{
    Win32Window window(1280, 720, "Win32WindowTest", false);

    window.SetMessageHook([](HWND, UINT, WPARAM, LPARAM) { return false; });

    bool resizeCallbackCalled = false;
    window.SetResizeCallback([&resizeCallbackCalled](int, int) { resizeCallbackCalled = true; });

    SetWindowPos(window.Handle(), nullptr, 0, 0, 800, 600, SWP_NOMOVE | SWP_NOZORDER);

    EXPECT_TRUE(resizeCallbackCalled);
}
