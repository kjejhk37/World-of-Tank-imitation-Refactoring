#include "renderer/RendererFactory.h"

#include "renderer/directx11/DirectX11Renderer.h"
#include "renderer/directx12/DirectX12Renderer.h"
#include "renderer/directx9/DirectX9Renderer.h"
#include "renderer/opengl/OpenGLRenderer.h"

namespace RendererFactory
{
    std::unique_ptr<IRenderer> Create(const LaunchConfig& config)
    {
        switch (config.backend)
        {
        case RendererBackend::DirectX9:
            return std::make_unique<DirectX9Renderer>();
        case RendererBackend::DirectX11:
            return std::make_unique<DirectX11Renderer>();
        case RendererBackend::DirectX12:
            return std::make_unique<DirectX12Renderer>();
        case RendererBackend::OpenGL:
            return std::make_unique<OpenGLRenderer>();
        }
        return nullptr;
    }
}
