#pragma once

// Author: Claude
// Description: 애플리케이션 전역 에러 코드. 카테고리별로 번호 대역을 나눠 향후 확장 시
//              기존 코드가 밀리지 않게 한다 (1000=Launch/Config, 2000=플랫폼/윈도우, 3000=렌더러).
//              새 카테고리가 생기면 4000번대부터 이어서 배정한다.
// Input: (해당 없음 - 상수 모음)
// Output: (해당 없음 - 상수 모음)
// Notes: main.cpp의 기존 4개 에러 처리 지점이 각각 아래 코드에 1:1로 대응된다.
// Date: 2026-07-19
enum class ErrorCode
{
    // 1000번대: 실행 인자 / Config 파싱
    LaunchConfigParseFailed = 1001,

    // 2000번대: 플랫폼 / 윈도우
    WindowCreationFailed = 2001,

    // 3000번대: 렌더러
    UnknownRendererBackend = 3001,
    RendererInitializationFailed = 3002,
};
