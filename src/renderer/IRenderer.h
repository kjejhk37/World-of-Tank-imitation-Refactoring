#pragma once

// Author: Claude
// Description: DirectX/OpenGL 초기화 절차를 감싸는 렌더러 인터페이스. 각 구현체가 외부 그래픽 API를 캡슐화한다.
// Input: (해당 없음 — 인터페이스)
// Output: (해당 없음 — 인터페이스)
// Notes: 이번 사이클에서는 구현체(DirectXRenderer/OpenGLRenderer)가 전부 스텁이다 — 실제 디바이스 초기화는 다음 사이클.
// Date: 2026-07-15
class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
};
