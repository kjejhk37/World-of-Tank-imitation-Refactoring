#include "renderer/directx11/DirectX11Renderer.h"

DirectX11Renderer::DirectX11Renderer(bool forceWarp)
    : m_forceWarp(forceWarp)
{
}

bool DirectX11Renderer::CreateDeviceAndSwapChain(D3D_DRIVER_TYPE driverType, HWND windowHandle, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = static_cast<UINT>(width);
    swapChainDesc.BufferDesc.Height = static_cast<UINT>(height);
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const HRESULT hr =
        D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
                                       m_swapChain.GetAddressOf(), m_device.GetAddressOf(), nullptr,
                                       m_context.GetAddressOf());
    return SUCCEEDED(hr);
}

bool DirectX11Renderer::CreateRenderTargetView()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
    return SUCCEEDED(hr);
}

bool DirectX11Renderer::Initialize(HWND windowHandle, int width, int height)
{
    bool created = false;
    if (!m_forceWarp)
    {
        created = CreateDeviceAndSwapChain(D3D_DRIVER_TYPE_HARDWARE, windowHandle, width, height);
    }
    if (!created)
    {
        created = CreateDeviceAndSwapChain(D3D_DRIVER_TYPE_WARP, windowHandle, width, height);
    }
    if (!created)
    {
        return false;
    }

    if (!CreateRenderTargetView())
    {
        return false;
    }

    m_uiManager = std::make_unique<ImGuiManagerDX11>(windowHandle, m_device.Get(), m_context.Get());
    return true;
}

void DirectX11Renderer::RenderFrame()
{
    if (!m_context || !m_renderTargetView)
    {
        return;
    }

    m_uiManager->NewFrame();

    constexpr float kClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), kClearColor);
    m_uiManager->Render();
    m_swapChain->Present(1, 0);
}

void DirectX11Renderer::OnResize(int width, int height)
{
    if (!m_swapChain || width <= 0 || height <= 0)
    {
        return;
    }

    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_renderTargetView.Reset();

    const HRESULT hr =
        m_swapChain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr))
    {
        CreateRenderTargetView();
    }
}

void DirectX11Renderer::Shutdown()
{
    if (m_uiManager)
    {
        m_uiManager->Shutdown();
        m_uiManager.reset();
    }
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_context.Reset();
    m_device.Reset();
}

bool DirectX11Renderer::HandleUiMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_uiManager && m_uiManager->HandleWin32Message(windowHandle, message, wParam, lParam);
}
