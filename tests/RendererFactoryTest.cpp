#include <gtest/gtest.h>

#include "config/LaunchConfig.h"
#include "renderer/directx11/DirectX11Renderer.h"
#include "renderer/directx12/DirectX12Renderer.h"
#include "renderer/directx9/DirectX9Renderer.h"
#include "renderer/opengl/OpenGLRenderer.h"
#include "renderer/RendererFactory.h"

TEST(RendererFactoryTest, CreatesDirectX9RendererForDirectX9Backend)
{
    const LaunchConfig config{RendererBackend::DirectX9};
    auto renderer = RendererFactory::Create(config);
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(dynamic_cast<DirectX9Renderer*>(renderer.get()), nullptr);
}

TEST(RendererFactoryTest, CreatesDirectX11RendererForDirectX11Backend)
{
    const LaunchConfig config{RendererBackend::DirectX11};
    auto renderer = RendererFactory::Create(config);
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(dynamic_cast<DirectX11Renderer*>(renderer.get()), nullptr);
}

TEST(RendererFactoryTest, CreatesDirectX12RendererForDirectX12Backend)
{
    const LaunchConfig config{RendererBackend::DirectX12};
    auto renderer = RendererFactory::Create(config);
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(dynamic_cast<DirectX12Renderer*>(renderer.get()), nullptr);
}

TEST(RendererFactoryTest, CreatesOpenGLRendererForOpenGLBackend)
{
    const LaunchConfig config{RendererBackend::OpenGL};
    auto renderer = RendererFactory::Create(config);
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(dynamic_cast<OpenGLRenderer*>(renderer.get()), nullptr);
}
