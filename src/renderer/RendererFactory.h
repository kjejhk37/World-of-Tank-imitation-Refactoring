#pragma once

#include <memory>

#include "config/LaunchConfig.h"
#include "renderer/IRenderer.h"

// Author: Claude
// Description: LaunchConfig에 담긴 RendererBackend에 따라 알맞은 IRenderer 구현체를 생성한다.
// Input: 생성 시점에 확정된 LaunchConfig
// Output: 선택된 백엔드에 대응하는 IRenderer 구현체의 소유권 있는 포인터
// Notes: 새 백엔드가 추가되면 이 팩토리에만 분기를 추가하면 되고, 호출부(main)는 변경할 필요가 없다.
// Date: 2026-07-15
namespace RendererFactory
{
    std::unique_ptr<IRenderer> Create(const LaunchConfig& config);
}
