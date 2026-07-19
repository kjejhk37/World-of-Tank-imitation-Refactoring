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
