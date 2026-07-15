#include "renderer/RendererFactory.h"

#include "renderer/DirectXRenderer.h"
#include "renderer/OpenGLRenderer.h"

namespace RendererFactory
{
    std::unique_ptr<IRenderer> Create(const LaunchConfig& config)
    {
        switch (config.backend)
        {
        case RendererBackend::DirectX:
            return std::make_unique<DirectXRenderer>();
        case RendererBackend::OpenGL:
            return std::make_unique<OpenGLRenderer>();
        }
        return nullptr;
    }
}
