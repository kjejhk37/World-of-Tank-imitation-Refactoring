#include <gtest/gtest.h>

#include "projects/config/LaunchConfigStore.h"

TEST(LaunchConfigStoreTest, GetReturnsWhatWasInitialized)
{
    LaunchConfigStore::Init(LaunchConfig{RendererBackend::OpenGL});
    EXPECT_EQ(LaunchConfigStore::Get().backend, RendererBackend::OpenGL);

    LaunchConfigStore::Init(LaunchConfig{RendererBackend::DirectX11});
    EXPECT_EQ(LaunchConfigStore::Get().backend, RendererBackend::DirectX11);
}
