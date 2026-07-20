#include "collision/fcl/Fcl.h"

// Author: Claude
// Description: Fcl 멤버 함수 구현 — Bvh로 후보 쌍을 추리고 GjkEpa로 확정한다.
// Input/Output: (Fcl.h 참고)
// Date: 2026-07-20

Fcl::Fcl(const std::vector<Geometry>& objects) : objects_(objects)
{
}

std::vector<CollisionPair> Fcl::DetectCollisions() const
{
    const Bvh bvh(objects_);
    const std::vector<std::pair<size_t, size_t>> candidates = bvh.FindOverlappingPairs();

    std::vector<CollisionPair> results;
    for (const auto& [indexA, indexB] : candidates)
    {
        const auto penetration = ComputePenetration(objects_[indexA], objects_[indexB]);
        if (penetration.has_value())
        {
            results.push_back(CollisionPair{indexA, indexB, *penetration});
        }
    }
    return results;
}
