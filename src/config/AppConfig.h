#pragma once

#include <string>

#include "renderer/RendererBackend.h"

// Author: Claude
// Description: JSON Config 파일에서 읽어오는 애플리케이션 설정값 모음.
// Input: (해당 없음 - 데이터 구조체)
// Output: (해당 없음 - 데이터 구조체)
// Notes: 필드 기본값은 Config 파일이 없거나 특정 필드가 누락된 경우 사용되는 하드코딩 기본값이다.
// Date: 2026-07-19
struct AppConfig
{
    static constexpr const char* kDefaultWindowTitle = "World of Tank Imitation";

    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    float volume = 1.0f;
    RendererBackend renderer = RendererBackend::DirectX11;
    std::string windowTitle = kDefaultWindowTitle;
};
