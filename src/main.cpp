#include <cstdio>
#include <stdexcept>

#include "config/LaunchConfigParser.h"
#include "config/LaunchConfigStore.h"
#include "renderer/RendererFactory.h"

// Author: Claude
// Description: 진입점. argv를 파싱해 LaunchConfigStore를 초기화하고, RendererFactory로 렌더러를 생성해 초기화한다.
// Input: argc, argv (OS가 전달하는 실행 인자)
// Output: 종료 코드 (0: 정상, 1: 인자 파싱 실패)
// Notes: LaunchConfigStore::Init은 반드시 이 함수 안에서, 다른 스레드가 뜨기 전에 1회만 호출되어야 한다.
// Date: 2026-07-15
int main(int argc, char** argv)
{
    try
    {
        LaunchConfigStore::Init(ParseLaunchConfig(argc, argv));
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
