#pragma once

// Author: Claude
// Description: math/geometry 전역에서 공유하는 상수 및 각도 단위 변환 함수.
//              이 헤더는 프로젝트의 다른 모듈(config/renderer/logging 등)을 전혀 참조하지 않는다 —
//              math/geometry를 나중에 별도 저장소(submodule)로 분리하기 쉽게 유지하기 위함.
// Input: (해당 없음 - 상수 모음 + 순수 함수)
// Output: (해당 없음 - 상수 모음 + 순수 함수)
// Notes: kEpsilon은 부동소수점 경계 비교(포함/충돌 판정 등)에 프로젝트 전역에서 일관되게 사용한다.
//        회전 관련 공개 API(Quaternion::FromAxisAngle 등)는 항상 Degree를 받고, 내부적으로만
//        DegreesToRadians로 변환해 sin/cos 등에 사용한다.
// Date: 2026-07-19
namespace MathConstants
{
    constexpr float kEpsilon = 1e-5f;
    constexpr float kPi = 3.14159265358979323846f;

    constexpr float DegreesToRadians(float degrees)
    {
        return degrees * (kPi / 180.0f);
    }

    constexpr float RadiansToDegrees(float radians)
    {
        return radians * (180.0f / kPi);
    }
}
