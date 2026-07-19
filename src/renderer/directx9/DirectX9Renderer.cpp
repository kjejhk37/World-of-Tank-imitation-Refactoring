#include "renderer/directx9/DirectX9Renderer.h"

namespace
{
    D3DPRESENT_PARAMETERS MakePresentParams(HWND windowHandle, int width, int height)
    {
        D3DPRESENT_PARAMETERS presentParams{};
        presentParams.Windowed = TRUE;
        presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
        presentParams.BackBufferFormat = D3DFMT_UNKNOWN;
        presentParams.hDeviceWindow = windowHandle;
        presentParams.BackBufferWidth = static_cast<UINT>(width);
        presentParams.BackBufferHeight = static_cast<UINT>(height);
        return presentParams;
    }
}

bool DirectX9Renderer::Initialize(HWND windowHandle, int width, int height)
{
    m_windowHandle = windowHandle;

    m_direct3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
    if (!m_direct3D)
    {
        return false;
    }

    D3DPRESENT_PARAMETERS presentParams = MakePresentParams(windowHandle, width, height);
    const HRESULT hr = m_direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
                                                 D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams,
                                                 m_device.GetAddressOf());
    return SUCCEEDED(hr);
}

void DirectX9Renderer::RenderFrame()
{
    if (!m_device)
    {
        return;
    }

    m_device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    m_device->BeginScene();
    m_device->EndScene();
    m_device->Present(nullptr, nullptr, nullptr, nullptr);
}

void DirectX9Renderer::OnResize(int width, int height)
{
    if (!m_device || width <= 0 || height <= 0)
    {
        return;
    }

    D3DPRESENT_PARAMETERS presentParams = MakePresentParams(m_windowHandle, width, height);
    m_device->Reset(&presentParams);
}

void DirectX9Renderer::Shutdown()
{
    m_device.Reset();
    m_direct3D.Reset();
}
