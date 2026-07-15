#include <gtest/gtest.h>

#include <stdexcept>

#include "config/LaunchConfigParser.h"

TEST(LaunchConfigParserTest, ParsesDirectXFlag)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=directx")};
    const LaunchConfig config = ParseLaunchConfig(2, argv);
    EXPECT_EQ(config.backend, RendererBackend::DirectX);
}

TEST(LaunchConfigParserTest, ParsesOpenGLFlag)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=opengl")};
    const LaunchConfig config = ParseLaunchConfig(2, argv);
    EXPECT_EQ(config.backend, RendererBackend::OpenGL);
}

TEST(LaunchConfigParserTest, DefaultsToDirectXWhenFlagMissing)
{
    char* argv[] = {const_cast<char*>("main")};
    const LaunchConfig config = ParseLaunchConfig(1, argv);
    EXPECT_EQ(config.backend, RendererBackend::DirectX);
}

TEST(LaunchConfigParserTest, ThrowsOnUnknownRendererValue)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=foo")};
    EXPECT_THROW(ParseLaunchConfig(2, argv), std::invalid_argument);
}
