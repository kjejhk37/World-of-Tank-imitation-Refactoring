#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "geometry/Geometry.h"

// Author: Claude
// Description: k-d Tree broad-phase. BVH와 트리 형태(이진 트리 + self-collide 쿼리)는 같지만, 분할 축을
//              "가장 넓은 축"이 아니라 깊이에 따라 순환(depth % 3)해서 고른다는 점이 유일하지만 본질적인
//              차이다(이름의 유래) — 참고용/낮은 우선순위 모듈(브레인스토밍 참고).
// Input: 생성자 - Geometry 목록
// Output: FindOverlappingPairs() - 목록 내 index 쌍(first < second), AABB가 겹치는 쌍만
// Notes: GJK/EPA/SAP/BVH/RTree/VoxelGrid/SpatialHash/Octree 등 다른 알고리즘 모듈을 참조하지 않는다
//        (독립성) — BVH와 구조가 닮았지만 이 파일 안에서 독립적으로 새로 구현했다.
// Date: 2026-07-20
class KdTree
{
public:
    explicit KdTree(const std::vector<Geometry>& objects);

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
                                        size_t end, int depth);
    static AABB Merge(const AABB& a, const AABB& b);
    static void SelfCollide(const Node* node, std::vector<std::pair<size_t, size_t>>& results);
    static void CollectOverlaps(const Node* a, const Node* b, std::vector<std::pair<size_t, size_t>>& results);

    std::unique_ptr<Node> root_;
};
