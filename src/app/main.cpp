#include <cstdio>
#include <memory>
#include <stdexcept>

#include "config/ConfigManager.h"
#include "config/LaunchConfigParser.h"
#include "config/LaunchConfigStore.h"
#include "platform/Win32Window.h"
#include "renderer/RendererFactory.h"
#include "serialization/JsonDataStore.h"

// 로컬 상수 분리 기준: 이 파일이 500줄을 넘거나 아래 상수가 5개를 넘으면
// 별도 헤더로 분리한다.
namespace
{
    constexpr const char* kDefaultConfigFilePath = "config.json";
}

// Author: Claude
// Description: 진입점. JSON Config 파일로 AppConfig를 읽고, argv로 이를 override해 LaunchConfigStore를 초기화한 뒤,
//              Win32Window와 RendererFactory로 선택된 렌더러를 초기화하고 빈 화면을 띄운 채 메시지 루프를 돈다.
// Input: argc, argv (OS가 전달하는 실행 인자)
// Output: 종료 코드 (0: 정상, 1: 인자 파싱/창 생성 실패 또는 렌더러 생성/초기화 실패)
// Notes: 설정 적용 순서는 하드코딩 기본값(AppConfig) -> config.json -> argv 순이다.
//        LaunchConfigStore::Init은 반드시 이 함수 안에서, 다른 스레드가 뜨기 전에 1회만 호출되어야 한다.
//        창 리사이즈는 Win32Window의 콜백을 통해 renderer->OnResize로 연결한다 — Win32Window는 IRenderer를 알지 못한다(SRP).
// Date: 2026-07-19
int main(int argc, char** argv)
{
    JsonDataStore configStore;
    const ConfigManager configManager(configStore);
    const AppConfig appConfig = configManager.LoadOrDefault(kDefaultConfigFilePath);

    try
    {
        LaunchConfigStore::Init(ParseLaunchConfig(argc, argv, appConfig.renderer));
    }
    catch (const std::invalid_argument& e)
    {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    std::unique_ptr<Win32Window> window;
    try
    {
        window = std::make_unique<Win32Window>(appConfig.width, appConfig.height, appConfig.windowTitle,
                                                 appConfig.fullscreen);
    }
    catch (const std::runtime_error& e)
    {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    auto renderer = RendererFactory::Create(LaunchConfigStore::Get());
    if (!renderer)
    {
        std::fprintf(stderr, "Unknown renderer backend\n");
        return 1;
    }

    if (!renderer->Initialize(window->Handle(), window->ClientWidth(), window->ClientHeight()))
    {
        std::fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }

    window->SetResizeCallback([&renderer](int width, int height) { renderer->OnResize(width, height); });

    while (window->PumpMessages())
    {
        renderer->RenderFrame();
    }

    renderer->Shutdown();

    return 0;
}
