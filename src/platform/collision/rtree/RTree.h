#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: R-Tree(Guttman, 1984 방식) broad-phase. BVH/Sweep and Prune과 같은 문제(겹칠 가능성이
//              있는 후보 쌍 추리기)를 풀지만, top-down으로 한 번에 짓는 BVH와 달리 Insert()로 점진적으로
//              자라는 동적 구조라는 점이 본질적 차이다.
// Input: 생성자(선택) - Geometry 목록 / Insert(object) - 추가할 Geometry 하나
// Output: FindOverlappingPairs() - 지금까지 삽입된 객체들의 (삽입 순서 기준) index 쌍(first < second),
//         AABB가 겹치는 쌍만
// Notes: GJK/EPA/SweepAndPrune/BVH 등 다른 알고리즘 모듈을 참조하지 않는다(독립성). 노드당 항목 수
//        상한(kMaxEntries)/하한(kMinEntries)은 고정값을 쓴다 — 동적 튜닝은 이번 사이클 범위 밖.
// Date: 2026-07-20
class RTree
{
public:
    RTree() = default;
    explicit RTree(const std::vector<Geometry>& objects);

    void Insert(const Geometry& object);
    std::vector<std::pair<size_t, size_t>> FindOverlappingPairs() const;

private:
    static constexpr size_t kMaxEntries = 4;
    static constexpr size_t kMinEntries = 2;

    struct Node;

    struct Entry
    {
        AABB bounds;
        std::unique_ptr<Node> child;  // 리프 항목이면 nullptr
        size_t objectIndex = 0;       // child == nullptr일 때만 의미 있음
    };

    struct Node
    {
        bool leaf = true;
        std::vector<Entry> entries;
    };

    void InsertLeafEntry(Entry entry);
    static std::unique_ptr<Node> InsertRecursive(Node* node, Entry entry);
    static std::unique_ptr<Node> SplitNode(Node* node);
    static AABB ComputeBounds(const std::vector<Entry>& entries);
    static size_t ChooseSubtree(const std::vector<Entry>& entries, const AABB& bounds);
    static void SelfCollide(const Node* node, std::vector<std::pair<size_t, size_t>>& results);
    static void CollectOverlaps(const Node* a, const Node* b, std::vector<std::pair<size_t, size_t>>& results);

    std::unique_ptr<Node> root_ = std::make_unique<Node>();
    size_t nextIndex_ = 0;
};
