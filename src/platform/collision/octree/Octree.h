#pragma once

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: Octree(적응형 8분할 트리) broad-phase. Voxel(균일 격자)과 달리 밀도에 따라 재귀적으로만
//              8분할한다 — 참고용/낮은 우선순위 모듈(브레인스토밍 참고, Voxel과 목적 중복).
// Input: 생성자 - Geometry 목록, 트리를 감쌀 worldBounds, 최대 깊이(maxDepth)
// Output: FindOverlappingPairs() - 목록 내 index 쌍(first < second), AABB가 겹치는 쌍만
// Notes: GJK/EPA/SAP/BVH/RTree/VoxelGrid/SpatialHash 등 다른 알고리즘 모듈을 참조하지 않는다(독립성).
//        객체는 "완전히 들어맞는 가장 깊은 자식"에만 배치한다(멀티 노드 등록 없음) — 어느 자식에도 완전히
//        들어맞지 않으면(중심선을 걸침) 현재 노드에 남긴다. 분할 시점 이전에 이미 저장된 객체는 재분배하지
//        않는다(정확성에는 영향 없음, 트리 품질에만 영향 — 참고용 모듈이라 허용하는 단순화).
// Date: 2026-07-20
class Octree
{
public:
    Octree(const std::vector<Geometry>& objects, const AABB& worldBounds, int maxDepth);

    std::vector<std::pair<size_t, size_t>> FindOverlappingPairs() const;

private:
    static constexpr size_t kMaxLocalBeforeSubdivide = 4;

    struct Node
    {
        AABB bounds;
        int depth = 0;
        bool subdivided = false;
        std::array<std::unique_ptr<Node>, 8> children;
        std::vector<size_t> localObjects;
    };

    void Insert(Node* node, size_t objectIndex, const AABB& bounds, int maxDepth);
    static void Subdivide(Node* node);
    static int FindContainingChild(const Node* node, const AABB& bounds);
    static void SelfCollide(const Node* node, const std::vector<AABB>& bounds,
                             std::vector<std::pair<size_t, size_t>>& results);
    static void CheckAgainstSubtree(const std::vector<size_t>& ancestorObjects, const Node* node,
                                     const std::vector<AABB>& bounds, std::vector<std::pair<size_t, size_t>>& results);

    std::unique_ptr<Node> root_;
    std::vector<AABB> objectBounds_;
};
