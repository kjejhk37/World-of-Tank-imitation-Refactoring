#include <gtest/gtest.h>

#include "platform/Win32Window.h"
#include "renderer/directx9/DirectX9Renderer.h"

// Note: 이 테스트는 실제 그래픽 드라이버(D3DDEVTYPE_HAL)가 있는 환경에서 실행된다고 가정한다.
// DX9의 소프트웨어 대체 경로(REF/NULLREF)는 레거시 SDK 컴포넌트가 있어야 안정적으로 동작해 자동 테스트에 쓰지 않는다.
TEST(DirectX9RendererTest, InitializeSucceedsOnRealHardware)
{
    Win32Window window(640, 480, "DirectX9RendererTest", false);

    DirectX9Renderer renderer;
    EXPECT_TRUE(renderer.Initialize(window.Handle(), 640, 480));
    renderer.Shutdown();
}

TEST(DirectX9RendererTest, SurvivesResizeAndRenderFrame)
{
    Win32Window window(640, 480, "DirectX9RendererTest", false);

    DirectX9Renderer renderer;
    ASSERT_TRUE(renderer.Initialize(window.Handle(), 640, 480));

    renderer.OnResize(800, 600);
    renderer.RenderFrame();

    renderer.OnResize(0, 0);  // 최소화 시나리오 — 크래시 없이 무시되어야 함
    renderer.RenderFrame();

    renderer.Shutdown();
}
