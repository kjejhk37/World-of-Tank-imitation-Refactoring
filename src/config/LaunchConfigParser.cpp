#include "config/LaunchConfigParser.h"

#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
    constexpr std::string_view kRendererFlagPrefix = "--renderer=";

    RendererBackend ParseRendererBackend(std::string_view value)
    {
        if (value == "directx")
        {
            return RendererBackend::DirectX;
        }
        if (value == "opengl")
        {
            return RendererBackend::OpenGL;
        }
        throw std::invalid_argument("Unknown --renderer value: " + std::string(value));
    }
}

LaunchConfig ParseLaunchConfig(int argc, char** argv)
{
    LaunchConfig config{RendererBackend::DirectX};

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];
        if (arg.rfind(kRendererFlagPrefix, 0) == 0)
        {
            config.backend = ParseRendererBackend(arg.substr(kRendererFlagPrefix.size()));
        }
    }

    return config;
}
