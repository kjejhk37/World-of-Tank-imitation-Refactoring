#pragma once

#include "renderer/IRenderer.h"

// Author: Claude
// Description: DirectX 렌더러 스텁. 이번 사이클은 백엔드 선택 구조 확보가 목적이라 실제 디바이스 초기화는 포함하지 않는다.
// Input: (해당 없음)
// Output: (해당 없음)
// Notes: Initialize()는 미구현 상태를 나타내는 값만 반환한다 — 다음 사이클(DirectX 디바이스 초기화)에서 실제 로직으로 대체 예정.
// Date: 2026-07-15
class DirectXRenderer final : public IRenderer
{
public:
    bool Initialize() override;
    void Shutdown() override;
};
