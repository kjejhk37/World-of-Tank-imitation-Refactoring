#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: BVH(Bounding Volume Hierarchy) broad-phase. Sweep and Prune과 같은 문제(겹칠 가능성이
//              있는 후보 쌍 추리기)를 트리 구조로 푼다 — 한 번 빌드해두고 여러 번 쿼리 가능하다는 점이
//              매 쿼리마다 새로 정렬하는 SAP과의 본질적 차이라 클래스로 설계했다.
// Input: 생성자 - Geometry 목록(빌드 시점에 한 번 GetBounds() 호출) / FindOverlappingPairs() - 없음
// Output: FindOverlappingPairs() - 생성자에 전달된 목록 내 index 쌍(first < second), AABB가 겹치는 쌍만
// Notes: GJK/EPA/SweepAndPrune 등 다른 알고리즘 모듈을 참조하지 않는다(독립성). 분할은 median split을
//        쓴다 — SAH 등 분할 품질 최적화는 이번 사이클 범위 밖.
// Date: 2026-07-20
class Bvh
{
public:
    explicit Bvh(const std::vector<Geometry>& objects);

    std::vector<std::pair<size_t, size_t>> FindOverlappingPairs() const;

private:
    struct Node
    {
        AABB bounds;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        size_t objectIndex = 0;

        bool IsLeaf() const
        {
            return left == nullptr && right == nullptr;
        }
    };

    static std::unique_ptr<Node> Build(std::vector<size_t>& indices, const std::vector<AABB>& bounds, size_t begin,
                                        size_t end);
    static AABB Merge(const AABB& a, const AABB& b);
    static void SelfCollide(const Node* node, std::vector<std::pair<size_t, size_t>>& results);
    static void CollectOverlaps(const Node* a, const Node* b, std::vector<std::pair<size_t, size_t>>& results);

    std::unique_ptr<Node> root_;
};
