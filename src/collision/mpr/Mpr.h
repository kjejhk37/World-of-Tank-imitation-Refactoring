#pragma once

#include "geometry/Geometry.h"

// Author: Claude
// Description: MPR(Minkowski Portal Refinement, XenoCollide 방식) narrow-phase. GJK와 같은 문제(두
//              Geometry의 충돌 여부)를 포탈 정제 방식으로 푼다 — GJK/EPA와 비교용, 낮은 우선순위 모듈.
// Input: MprIntersects(a, b) - 비교할 두 Geometry
// Output: 충돌 여부(bool). 침투 깊이는 이번 모듈 범위 밖(EPA에서 이미 확보).
// Notes: Geometry.h의 Support() 자유 함수만 사용한다(접근 퍼사드 규칙). GJK/EPA 등 다른 알고리즘 모듈을
//        참조하지 않는다(독립성) — 개념적 비교 대상일 뿐 코드는 완전히 별개. 이름을 `Intersects`가 아닌
//        `MprIntersects`로 둔 이유: GjkEpa.h가 이미 전역에 동일 시그니처의 `Intersects(Geometry,Geometry)`를
//        정의하고 있어(app_lib에 두 .cpp가 함께 링크되므로), 이름이 같으면 링크 단계에서 중복 정의 오류가 난다.
//        알려진 한계: OBB처럼 지지 함수가 한 옥탄트 전체에서 같은 모서리를 반환하는 평평한(polytope)
//        도형에서는 포탈 정제가 수렴하지 않고 몇 개의 정점 사이를 순환할 수 있다(Sphere/AABB 조합만
//        검증됨 — docs/commit/MPR_20260720_2259.md 참고).
// Date: 2026-07-20
bool MprIntersects(const Geometry& a, const Geometry& b);
