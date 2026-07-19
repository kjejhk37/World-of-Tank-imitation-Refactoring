#pragma once

#include <wrl/client.h>

#include <array>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "renderer/IRenderer.h"

// Author: Claude
// Description: IRenderer의 DirectX 12 구현. ID3D12Device 등 D3D12 API 호출은 이 클래스(.h/.cpp) 안에만 존재한다.
// Input: 생성자 - forceWarp(테스트용, 기본 false) / Initialize - 렌더링 대상 HWND, 클라이언트 영역 너비/높이
// Output: (해당 없음 - IRenderer 인터페이스 구현)
// Notes: 기본적으로 하드웨어 어댑터로 디바이스 생성을 시도하고, 실패하면 WARP 어댑터로 폴백한다(DX11과 동일한 실사용 목적 폴백).
//        forceWarp=true면 하드웨어를 건너뛰고 WARP 어댑터만 시도 — 자동 테스트에서 GPU 유무와 무관하게 디바이스 생성을 검증하기 위함.
//        kCommandListCount는 확장성 가이드라인(브레인스토밍 참고) — 지금은 1(싱글스레드 기록)이지만, 나중에 커맨드 리스트를
//        여러 스레드에서 병렬 기록하도록 확장할 때 이 상수와 관련 컨테이너 크기만 늘리면 되게 구조를 열어둔다.
//        매 프레임 Present 직후 펜스로 GPU를 완전히 대기(WaitForGpu)한다 — 프레임 파이프라이닝 최적화는 이번 사이클 범위 밖.
// Date: 2026-07-19
class DirectX12Renderer final : public IRenderer
{
public:
    explicit DirectX12Renderer(bool forceWarp = false);
    ~DirectX12Renderer() override;

    bool Initialize(HWND windowHandle, int width, int height) override;
    void RenderFrame() override;
    void OnResize(int width, int height) override;
    void Shutdown() override;

private:
    static constexpr UINT kBackBufferCount = 2;   // DX12 flip-model 스왑체인의 최소 버퍼 요구사항
    static constexpr UINT kCommandListCount = 1;  // 확장성 가이드라인 — 나중에 멀티스레드 기록 시 이 값만 늘리면 됨

    bool m_forceWarp;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kBackBufferCount> m_renderTargets;
    std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, kCommandListCount> m_commandAllocators;
    std::array<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>, kCommandListCount> m_commandLists;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    UINT64 m_fenceValue = 0;
    UINT m_rtvDescriptorSize = 0;
    UINT m_frameIndex = 0;

    bool CreateDevice();
    bool CreateSwapChain(HWND windowHandle, int width, int height);
    bool CreateRenderTargets();
    void ReleaseRenderTargets();
    void WaitForGpu();
};
