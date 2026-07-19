#pragma once

#include <wrl/client.h>

#include <memory>

#include <d3d11.h>
#include <dxgi.h>

#include "renderer/IRenderer.h"
#include "ui/directx11/ImGuiManagerDX11.h"

// Author: Claude
// Description: IRenderer의 DirectX 11 구현. ID3D11Device/IDXGISwapChain 등 D3D11 API 호출은 이 클래스(.h/.cpp) 안에만 존재한다.
// Input: 생성자 - forceWarp(테스트용, 기본 false) / Initialize - 렌더링 대상 HWND, 클라이언트 영역 너비/높이
// Output: (해당 없음 - IRenderer 인터페이스 구현)
// Notes: 기본적으로 하드웨어 드라이버로 디바이스 생성을 시도하고, 실패하면 WARP(소프트웨어 래스터라이저)로 폴백한다
//        (GPU/드라이버가 없는 환경에서도 최소한 동작하게 하기 위한 실사용 목적의 폴백 — 테스트 전용이 아니다).
//        forceWarp=true로 생성하면 하드웨어를 건너뛰고 WARP만 시도한다 — 자동 테스트에서 GPU 유무와 무관하게 디바이스 생성을 검증하기 위함.
//        디바이스 상태는 영속 멤버로 보유해 다음 사이클(실제 렌더링 기능)에서 재사용 가능하게 한다.
//        ImGuiManagerDX11을 멤버로 소유해 ImGui 프레임워크(UI 오버레이)를 배선한다 — 이번 사이클은 프레임워크
//        확보만 목표라 실제 위젯은 그리지 않는다.
// Date: 2026-07-19
class DirectX11Renderer final : public IRenderer
{
public:
    explicit DirectX11Renderer(bool forceWarp = false);

    bool Initialize(HWND windowHandle, int width, int height) override;
    void RenderFrame() override;
    void OnResize(int width, int height) override;
    void Shutdown() override;
    bool HandleUiMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    bool m_forceWarp;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    std::unique_ptr<ImGuiManagerDX11> m_uiManager;

    bool CreateDeviceAndSwapChain(D3D_DRIVER_TYPE driverType, HWND windowHandle, int width, int height);
    bool CreateRenderTargetView();
};
