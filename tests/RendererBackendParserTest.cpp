#include <gtest/gtest.h>

#include "renderer/RendererBackendParser.h"

TEST(RendererBackendParserTest, ParsesKnownValues)
{
    EXPECT_EQ(RendererBackendParser::TryParse("directx"), RendererBackend::DirectX);
    EXPECT_EQ(RendererBackendParser::TryParse("opengl"), RendererBackend::OpenGL);
}

TEST(RendererBackendParserTest, ReturnsNulloptForUnknownValue)
{
    EXPECT_FALSE(RendererBackendParser::TryParse("foo").has_value());
}
