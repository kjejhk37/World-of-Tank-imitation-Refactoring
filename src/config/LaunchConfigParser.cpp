#include "config/LaunchConfigParser.h"

#include <stdexcept>
#include <string>
#include <string_view>

#include "renderer/RendererBackendParser.h"

namespace
{
    constexpr std::string_view kRendererFlagPrefix = "--renderer=";
}

LaunchConfig ParseLaunchConfig(int argc, char** argv, RendererBackend defaultBackend)
{
    LaunchConfig config{defaultBackend};

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];
        if (arg.rfind(kRendererFlagPrefix, 0) == 0)
        {
            const std::string_view value = arg.substr(kRendererFlagPrefix.size());
            const auto parsed = RendererBackendParser::TryParse(value);
            if (!parsed)
            {
                throw std::invalid_argument("Unknown --renderer value: " + std::string(value));
            }
            config.backend = *parsed;
        }
    }

    return config;
}
