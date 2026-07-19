#pragma once

// Author: Claude
// Description: config.json 스키마의 필드 이름 상수 모음. ConfigManager가 DataRecord에서 값을 읽을 때 사용한다.
// Input: (해당 없음 - 상수 모음)
// Output: (해당 없음 - 상수 모음)
// Notes: ConfigManager.cpp에 인라인으로 두지 않고 별도 헤더로 분리한 이유 — 필드가 5개 이상이라 앞으로도 늘어날 여지가 있고,
//        config.json의 전체 스키마를 이 파일 하나만 보고 파악할 수 있게 하기 위함.
// Date: 2026-07-19
namespace ConfigFields
{
    constexpr const char* kWidth = "width";
    constexpr const char* kHeight = "height";
    constexpr const char* kFullscreen = "fullscreen";
    constexpr const char* kVolume = "volume";
    constexpr const char* kRenderer = "renderer";
    constexpr const char* kWindowTitle = "windowTitle";
}
