#pragma once

#include <cstdint>
#include <vector>

#include "math/Vec3.h"

// Author: Claude
// Description: 삼각형 메시의 순수 데이터 표현(정점 목록 + 인덱스 목록).
// Input: (해당 없음 - 데이터 구조체)
// Output: (해당 없음 - 데이터 구조체)
// Notes: indices는 3개씩 묶여 삼각형 하나를 이룬다(삼각형 리스트).
//        Mesh 기반 충돌 검사는 이번 사이클 범위 밖이다 — 더 복잡한 별도 방법(예: BVH)으로 다음 사이클에서
//        다룰 예정이라 이 클래스는 데이터 표현만 담당한다.
// Date: 2026-07-19
struct Mesh
{
    std::vector<Vec3> vertices;
    std::vector<uint32_t> indices;
};
