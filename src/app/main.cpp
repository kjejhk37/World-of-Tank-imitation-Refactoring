#include <cstdio>
#include <stdexcept>

#include "config/ConfigManager.h"
#include "config/LaunchConfigParser.h"
#include "config/LaunchConfigStore.h"
#include "renderer/RendererFactory.h"
#include "serialization/JsonDataStore.h"

// 로컬 상수 분리 기준: 이 파일이 500줄을 넘거나 아래 상수가 5개를 넘으면
// 별도 헤더로 분리한다.
namespace
{
    constexpr const char* kDefaultConfigFilePath = "config.json";
}

// Author: Claude
// Description: 진입점. JSON Config 파일로 AppConfig를 읽고, argv로 이를 override해 LaunchConfigStore를 초기화한 뒤, RendererFactory로 렌더러를 생성해 초기화한다.
// Input: argc, argv (OS가 전달하는 실행 인자)
// Output: 종료 코드 (0: 정상, 1: 인자 파싱 실패)
// Notes: 설정 적용 순서는 하드코딩 기본값(AppConfig) -> config.json -> argv 순이다.
//        LaunchConfigStore::Init은 반드시 이 함수 안에서, 다른 스레드가 뜨기 전에 1회만 호출되어야 한다.
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

    auto renderer = RendererFactory::Create(LaunchConfigStore::Get());
    renderer->Initialize();

    return 0;
}
