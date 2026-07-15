#include <gtest/gtest.h>

#include "config/LaunchConfig.h"
#include "renderer/DirectXRenderer.h"
#include "renderer/OpenGLRenderer.h"
#include "renderer/RendererFactory.h"

TEST(RendererFactoryTest, CreatesDirectXRendererForDirectXBackend)
{
    const LaunchConfig config{RendererBackend::DirectX};
    auto renderer = RendererFactory::Create(config);
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(dynamic_cast<DirectXRenderer*>(renderer.get()), nullptr);
}

TEST(RendererFactoryTest, CreatesOpenGLRendererForOpenGLBackend)
{
    const LaunchConfig config{RendererBackend::OpenGL};
    auto renderer = RendererFactory::Create(config);
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(dynamic_cast<OpenGLRenderer*>(renderer.get()), nullptr);
}
