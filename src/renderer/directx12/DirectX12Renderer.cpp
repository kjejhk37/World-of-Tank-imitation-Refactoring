#include "renderer/directx12/DirectX12Renderer.h"

DirectX12Renderer::DirectX12Renderer(bool forceWarp)
    : m_forceWarp(forceWarp)
{
}

DirectX12Renderer::~DirectX12Renderer()
{
    Shutdown();
}

bool DirectX12Renderer::CreateDevice()
{
    if (!m_forceWarp &&
        SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.GetAddressOf()))))
    {
        return true;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()))))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
    if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.GetAddressOf()))))
    {
        return false;
    }

    return SUCCEEDED(
        D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.GetAddressOf())));
}

bool DirectX12Renderer::CreateSwapChain(HWND windowHandle, int width, int height)
{
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.GetAddressOf()))))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()))))
    {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.BufferCount = kBackBufferCount;
    swapChainDesc.Width = static_cast<UINT>(width);
    swapChainDesc.Height = static_cast<UINT>(height);
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(m_commandQueue.Get(), windowHandle, &swapChainDesc, nullptr, nullptr,
                                                swapChain1.GetAddressOf())))
    {
        return false;
    }

    if (FAILED(swapChain1.As(&m_swapChain)))
    {
        return false;
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool DirectX12Renderer::CreateRenderTargets()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = kBackBufferCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf()))))
    {
        return false;
    }

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kBackBufferCount; ++i)
    {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].GetAddressOf()))))
        {
            return false;
        }
        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
    return true;
}

void DirectX12Renderer::ReleaseRenderTargets()
{
    for (auto& renderTarget : m_renderTargets)
    {
        renderTarget.Reset();
    }
}

void DirectX12Renderer::WaitForGpu()
{
    if (!m_commandQueue || !m_fence)
    {
        return;
    }

    const UINT64 fenceValueToSignal = m_fenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceValueToSignal);
    ++m_fenceValue;

    if (m_fence->GetCompletedValue() < fenceValueToSignal)
    {
        m_fence->SetEventOnCompletion(fenceValueToSignal, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

bool DirectX12Renderer::Initialize(HWND windowHandle, int width, int height)
{
    if (!CreateDevice() || !CreateSwapChain(windowHandle, width, height) || !CreateRenderTargets())
    {
        return false;
    }

    for (UINT i = 0; i < kCommandListCount; ++i)
    {
        if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                      IID_PPV_ARGS(m_commandAllocators[i].GetAddressOf()))))
        {
            return false;
        }
    }

    if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr,
                                            IID_PPV_ARGS(m_commandLists[0].GetAddressOf()))))
    {
        return false;
    }
    m_commandLists[0]->Close();  // 초기 상태를 닫힌 상태로 맞춰 RenderFrame의 Reset() 호출과 짝을 맞춘다

    if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()))))
    {
        return false;
    }
    m_fenceValue = 1;
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    return m_fenceEvent != nullptr;
}

void DirectX12Renderer::RenderFrame()
{
    if (!m_commandQueue || !m_swapChain)
    {
        return;
    }

    auto& commandAllocator = m_commandAllocators[0];
    auto& commandList = m_commandLists[0];

    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);

    D3D12_RESOURCE_BARRIER toRenderTarget{};
    toRenderTarget.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toRenderTarget.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    toRenderTarget.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    toRenderTarget.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    toRenderTarget.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &toRenderTarget);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(m_frameIndex) * m_rtvDescriptorSize;

    constexpr float kClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    commandList->ClearRenderTargetView(rtvHandle, kClearColor, 0, nullptr);

    D3D12_RESOURCE_BARRIER toPresent{};
    toPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toPresent.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    toPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    toPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    toPresent.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &toPresent);

    commandList->Close();

    ID3D12CommandList* commandListsToExecute[] = {commandList.Get()};
    m_commandQueue->ExecuteCommandLists(1, commandListsToExecute);

    m_swapChain->Present(1, 0);

    WaitForGpu();
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DirectX12Renderer::OnResize(int width, int height)
{
    if (!m_swapChain || width <= 0 || height <= 0)
    {
        return;
    }

    WaitForGpu();
    ReleaseRenderTargets();

    const HRESULT hr = m_swapChain->ResizeBuffers(kBackBufferCount, static_cast<UINT>(width),
                                                    static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0);
    if (SUCCEEDED(hr))
    {
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        CreateRenderTargets();
    }
}

void DirectX12Renderer::Shutdown()
{
    WaitForGpu();

    if (m_fenceEvent != nullptr)
    {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    ReleaseRenderTargets();
    m_rtvHeap.Reset();
    for (auto& commandList : m_commandLists)
    {
        commandList.Reset();
    }
    for (auto& commandAllocator : m_commandAllocators)
    {
        commandAllocator.Reset();
    }
    m_fence.Reset();
    m_swapChain.Reset();
    m_commandQueue.Reset();
    m_device.Reset();
}
