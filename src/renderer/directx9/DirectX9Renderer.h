#pragma once

#include <wrl/client.h>

#include <memory>

#include <d3d9.h>

#include "renderer/IRenderer.h"
#include "ui/directx9/ImGuiManagerDX9.h"

// Author: Claude
// Description: IRenderer의 DirectX 9 구현. IDirect3D9/IDirect3DDevice9 API 호출은 이 클래스(.h/.cpp) 안에만 존재한다.
// Input: 생성자 - (해당 없음) / Initialize - 렌더링 대상 HWND, 클라이언트 영역 너비/높이
// Output: (해당 없음 - IRenderer 인터페이스 구현)
// Notes: DX9은 DXGI를 쓰지 않는 세대라 D3DPRESENT_PARAMETERS로 암시적 스왑체인을 구성한다.
//        DX11/12의 WARP와 달리, DX9의 REF/NULLREF 소프트웨어 디바이스는 별도 레거시 SDK 컴포넌트가 있어야 안정적으로 동작해
//        이 클래스는 하드웨어(D3DDEVTYPE_HAL) 생성만 시도한다 — 실패 시 false를 반환한다.
//        디바이스 상태(IDirect3D9/IDirect3DDevice9)는 영속 멤버로 보유해 다음 사이클(실제 렌더링 기능)에서 재사용 가능하게 한다.
//        ImGuiManagerDX9을 멤버로 소유해 ImGui 프레임워크(UI 오버레이)를 배선한다 — 이번 사이클은 프레임워크
//        확보만 목표라 실제 위젯은 그리지 않는다(Render는 매 프레임 호출되지만 그 사이에 그려지는 콘텐츠 없음).
// Date: 2026-07-19
class DirectX9Renderer final : public IRenderer
{
public:
    bool Initialize(HWND windowHandle, int width, int height) override;
    void RenderFrame() override;
    void OnResize(int width, int height) override;
    void Shutdown() override;
    bool HandleUiMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    Microsoft::WRL::ComPtr<IDirect3D9> m_direct3D;
    Microsoft::WRL::ComPtr<IDirect3DDevice9> m_device;
    HWND m_windowHandle = nullptr;
    std::unique_ptr<ImGuiManagerDX9> m_uiManager;
};
