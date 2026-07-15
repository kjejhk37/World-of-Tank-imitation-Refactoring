#pragma once

#include "config/LaunchConfig.h"

// Author: Claude
// Description: 프로세스 전역에서 접근 가능한 LaunchConfig 저장소 (namespace 기반, GoF Singleton 클래스 아님).
// Input: (Init) 파싱된 LaunchConfig 값 / (Get) 없음
// Output: (Init) 없음 / (Get) 저장된 LaunchConfig의 const 참조
// Notes: Init은 main() 진입 직후, 다른 스레드가 뜨기 전에 단 한 번만 호출해야 한다.
//        그 이후로는 값을 재설정하지 않는 "쓰기 후 불변" 계약을 가정하므로, 여러 스레드에서 Get()을 동시에 호출해도 안전하다.
//        Init 이전에 Get()을 호출하면 미정의 동작(초기화되지 않은 값)이다.
// Date: 2026-07-15
namespace LaunchConfigStore
{
    void Init(LaunchConfig config);
    const LaunchConfig& Get();
}
