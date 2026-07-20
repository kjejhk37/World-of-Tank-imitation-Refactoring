#pragma once

#include "geometry/Geometry.h"
#include "math/Vec3.h"

// Author: Claude
// Description: 두 Geometry의 민코프스키 차(Minkowski Difference) A-B 위에서의 지지점(support point)을
//              계산하는 내부 헬퍼. GJK/EPA가 공통으로 사용하는 계산이라 별도 헤더로 뺐다.
// Input: a, b - 비교할 두 도형 / direction - 탐색 방향(정규화 불필요)
// Output: A-B(민코프스키 차) 위에서 direction으로 가장 먼 점
// Notes: Geometry.h의 Support() 자유 함수만 사용한다(접근 퍼사드 규칙 준수) — std::visit/get 직접 호출 없음.
//        이 헤더는 src/collision/gjk_epa 내부 전용이다(Gjk.cpp/Epa.cpp만 include) — 외부 공개 API가 아니다.
// Date: 2026-07-20
inline Vec3 MinkowskiSupport(const Geometry& a, const Geometry& b, const Vec3& direction)
{
    return Support(a, direction) - Support(b, -direction);
}
