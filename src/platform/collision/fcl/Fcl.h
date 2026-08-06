#pragma once

#include <cstddef>
#include <vector>

#include "platform/collision/bvh/Bvh.h"
#include "platform/collision/gjk_epa/GjkEpa.h"
#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: 커스텀 FCL(Flexible Collision Library) 스타일 통합 파이프라인 — broad-phase(BVH)로
//              후보 쌍을 값싸게 추린 뒤, narrow-phase(GJK+EPA)로 실제 충돌 여부와 침투 정보를 확정한다.
// Input: 생성자 - Geometry 목록 / DetectCollisions() - 없음
// Output: DetectCollisions() - 실제로 충돌하는 쌍(index 쌍 + 침투 법선/깊이) 목록
// Notes: 요구사항 1(모든 충돌 감지 알고리즘은 서로 독립)의 명시적 예외 — 이 모듈만 Bvh/GjkEpa 두 모듈을
//        직접 참조한다(브레인스토밍에서 합의됨). 다른 모듈(SAP/RTree/Voxel/SpatialHash/Octree/KdTree/
//        MPR/CCD)은 참조하지 않는다 — 이들이 먼저 완성되어 있어야 하므로 로드맵상 항상 마지막에 온다.
// Date: 2026-07-20
struct CollisionPair
{
    size_t indexA;
    size_t indexB;
    PenetrationInfo penetration;
};

class Fcl
{
public:
    explicit Fcl(const std::vector<Geometry>& objects);

    std::vector<CollisionPair> DetectCollisions() const;

private:
    std::vector<Geometry> objects_;
};
