#pragma once

#include "renderer/RendererBackend.h"

// Author: Claude
// Description: argv 파싱 결과를 담는 불변 값 모음. LaunchConfigStore를 통해 프로세스 전역에서 읽힌다.
// Input: (해당 없음 — 데이터 구조체)
// Output: (해당 없음 — 데이터 구조체)
// Notes: 필드가 늘어나도 이 struct 하나만 확장하면 되고, 소비자(RendererFactory 등)의 시그니처는 바뀌지 않는다.
// Date: 2026-07-15
struct LaunchConfig
{
    RendererBackend backend;
};
