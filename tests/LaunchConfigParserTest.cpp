#include <gtest/gtest.h>

#include <stdexcept>

#include "projects/config/LaunchConfigParser.h"

TEST(LaunchConfigParserTest, ParsesDirectX11Flag)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=directx11")};
    const LaunchConfig config = ParseLaunchConfig(2, argv);
    EXPECT_EQ(config.backend, RendererBackend::DirectX11);
}

TEST(LaunchConfigParserTest, ParsesDirectX12Flag)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=directx12")};
    const LaunchConfig config = ParseLaunchConfig(2, argv);
    EXPECT_EQ(config.backend, RendererBackend::DirectX12);
}

TEST(LaunchConfigParserTest, ParsesDirectX9Flag)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=directx9")};
    const LaunchConfig config = ParseLaunchConfig(2, argv);
    EXPECT_EQ(config.backend, RendererBackend::DirectX9);
}

TEST(LaunchConfigParserTest, ParsesOpenGLFlag)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=opengl")};
    const LaunchConfig config = ParseLaunchConfig(2, argv);
    EXPECT_EQ(config.backend, RendererBackend::OpenGL);
}

TEST(LaunchConfigParserTest, DefaultsToDirectX11WhenFlagMissing)
{
    char* argv[] = {const_cast<char*>("main")};
    const LaunchConfig config = ParseLaunchConfig(1, argv);
    EXPECT_EQ(config.backend, RendererBackend::DirectX11);
}

TEST(LaunchConfigParserTest, ThrowsOnUnknownRendererValue)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=foo")};
    EXPECT_THROW(ParseLaunchConfig(2, argv), std::invalid_argument);
}

TEST(LaunchConfigParserTest, UsesProvidedDefaultBackendWhenFlagMissing)
{
    char* argv[] = {const_cast<char*>("main")};
    const LaunchConfig config = ParseLaunchConfig(1, argv, RendererBackend::OpenGL);
    EXPECT_EQ(config.backend, RendererBackend::OpenGL);
}

TEST(LaunchConfigParserTest, ArgvOverridesProvidedDefaultBackend)
{
    char* argv[] = {const_cast<char*>("main"), const_cast<char*>("--renderer=directx11")};
    const LaunchConfig config = ParseLaunchConfig(2, argv, RendererBackend::OpenGL);
    EXPECT_EQ(config.backend, RendererBackend::DirectX11);
}
