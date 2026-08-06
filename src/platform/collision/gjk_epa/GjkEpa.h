#pragma once

#include <optional>

#include "platform/collision/gjk_epa/Epa.h"
#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: GJK+EPA 모듈의 공개 API. 다른 알고리즘 모듈(BVH/R-Tree 등)이 접근해야 할 유일한 진입점.
// Input: Intersects(a,b)/ComputePenetration(a,b) - 비교할 두 Geometry
// Output: Intersects - 충돌 여부(bool) / ComputePenetration - 충돌 시 PenetrationInfo, 아니면 std::nullopt
// Notes: 내부적으로 RunGjk() 실행 후 충돌이면 RunEpa()를 이어서 호출한다. Gjk.h/Epa.h는 이 모듈 내부
//        구현 세부사항이다 — 다른 모듈은 이 헤더(GjkEpa.h)만 include해야 한다.
// Date: 2026-07-20
bool Intersects(const Geometry& a, const Geometry& b);
std::optional<PenetrationInfo> ComputePenetration(const Geometry& a, const Geometry& b);
