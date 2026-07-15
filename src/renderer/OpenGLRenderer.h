#pragma once

#include "renderer/IRenderer.h"

// Author: Claude
// Description: OpenGL 렌더러 스텁. 현재 프로젝트 목표는 DirectX 마이그레이션이며, OpenGL은 향후 확장 대비용 선택지다.
// Input: (해당 없음)
// Output: (해당 없음)
// Notes: Initialize()는 미구현 상태를 나타내는 값만 반환한다 — OpenGL 실사용은 이번 범위에 포함되지 않는다.
// Date: 2026-07-15
class OpenGLRenderer final : public IRenderer
{
public:
    bool Initialize() override;
    void Shutdown() override;
};
