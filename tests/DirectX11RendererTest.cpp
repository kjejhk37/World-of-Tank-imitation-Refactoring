#include <gtest/gtest.h>

#include "platform/Win32Window.h"
#include "renderer/directx11/DirectX11Renderer.h"

TEST(DirectX11RendererTest, InitializeSucceedsWithForcedWarp)
{
    Win32Window window(640, 480, "DirectX11RendererTest", false);

    DirectX11Renderer renderer(/*forceWarp=*/true);
    EXPECT_TRUE(renderer.Initialize(window.Handle(), 640, 480));
    renderer.Shutdown();
}

TEST(DirectX11RendererTest, InitializeSucceedsWithDefaultDriverSelection)
{
    Win32Window window(640, 480, "DirectX11RendererTest", false);

    DirectX11Renderer renderer;
    EXPECT_TRUE(renderer.Initialize(window.Handle(), 640, 480));
    renderer.Shutdown();
}

TEST(DirectX11RendererTest, SurvivesResizeAndRenderFrame)
{
    Win32Window window(640, 480, "DirectX11RendererTest", false);

    DirectX11Renderer renderer(/*forceWarp=*/true);
    ASSERT_TRUE(renderer.Initialize(window.Handle(), 640, 480));

    renderer.OnResize(800, 600);
    renderer.RenderFrame();

    renderer.OnResize(0, 0);  // 최소화 시나리오 — 크래시 없이 무시되어야 함
    renderer.RenderFrame();

    renderer.Shutdown();
}
