#include "config/LaunchConfigStore.h"

namespace
{
    LaunchConfig g_launchConfig{RendererBackend::DirectX11};
}

namespace LaunchConfigStore
{
    void Init(LaunchConfig config)
    {
        g_launchConfig = config;
    }

    const LaunchConfig& Get()
    {
        return g_launchConfig;
    }
}
