#pragma once

#include "geometry/Geometry.h"

// Author: Claude
// Description: Conservative Advancement(연속 충돌 감지, CCD). 선속도를 가진 두 Geometry가 deltaTime
//              동안 이동할 때 충돌 여부와 충돌 시각(Time of Impact)을 구한다 — 이산 알고리즘(GJK/EPA
//              등)이 놓칠 수 있는 고속 이동체의 터널링을 방지한다.
// Input: ConservativeAdvancement(a, velocityA, b, velocityB, deltaTime)
// Output: ToiResult - collided(충돌 여부) + timeOfImpact(충돌 시 그 시각, [0, deltaTime])
// Notes: GJK/EPA 등 다른 알고리즘 모듈을 참조하지 않는다(독립성). 실제 도형 대신 GetBounds()의 경계 구
//        (중심=AABB 중심, 반지름=AABB half-diagonal)로 거리를 근사한다 — 경계 구 사이 거리는 항상 실제
//        도형 사이 최단 거리의 안전한(보수적인) 하한이라 "아직 안 닿았다"는 판단은 항상 유효하다. 정확한
//        지지 함수 기반 최단 거리 쿼리는 별도의 서브 알고리즘이 필요해 이번 사이클 범위 밖이다.
// Date: 2026-07-20
struct ToiResult
{
    bool collided = false;
    float timeOfImpact = 0.0f;
};

ToiResult ConservativeAdvancement(const Geometry& a, const Vec3& velocityA, const Geometry& b, const Vec3& velocityB,
                                   float deltaTime);
