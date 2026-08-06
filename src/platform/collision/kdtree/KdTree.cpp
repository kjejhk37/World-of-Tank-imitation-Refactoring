#include "platform/collision/kdtree/KdTree.h"

#include <algorithm>

#include "platform/geometry/Intersections.h"

// Author: Claude
// Description: KdTree 멤버 함수 구현 — 깊이 순환 축 + 중앙값 분할 빌드, self-collide 쿼리는 BVH와 같은
//              패턴(이 파일 안에서 독립적으로 구현).
// Input/Output: (KdTree.h 참고)
// Date: 2026-07-20

KdTree::KdTree(const std::vector<Geometry>& objects)
{
    std::vector<AABB> bounds;
    bounds.reserve(objects.size());
    for (const Geometry& object : objects)
    {
        bounds.push_back(GetBounds(object));
    }

    std::vector<size_t> indices(objects.size());
    for (size_t i = 0; i < indices.size(); ++i)
    {
        indices[i] = i;
    }

    if (!indices.empty())
    {
        root_ = Build(indices, bounds, 0, indices.size(), 0);
    }
}

AABB KdTree::Merge(const AABB& a, const AABB& b)
{
    return AABB(Vec3(std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z)),
                Vec3(std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z)));
}

std::unique_ptr<KdTree::Node> KdTree::Build(std::vector<size_t>& indices, const std::vector<AABB>& bounds,
                                             size_t begin, size_t end, int depth)
{
    auto node = std::make_unique<Node>();

    if (end - begin == 1)
    {
        node->bounds = bounds[indices[begin]];
        node->objectIndex = indices[begin];
        return node;
    }

    const int axis = depth % 3;  // k-d Tree의 정체성: 축을 깊이에 따라 순환한다(BVH는 넓은 축을 고름).
    const size_t mid = begin + (end - begin) / 2;
    std::nth_element(indices.begin() + static_cast<std::ptrdiff_t>(begin),
                      indices.begin() + static_cast<std::ptrdiff_t>(mid),
                      indices.begin() + static_cast<std::ptrdiff_t>(end), [&](size_t lhs, size_t rhs) {
                          return bounds[lhs].Center()[axis] < bounds[rhs].Center()[axis];
                      });

    node->left = Build(indices, bounds, begin, mid, depth + 1);
    node->right = Build(indices, bounds, mid, end, depth + 1);
    node->bounds = Merge(node->left->bounds, node->right->bounds);
    return node;
}

void KdTree::CollectOverlaps(const Node* a, const Node* b, std::vector<std::pair<size_t, size_t>>& results)
{
    if (!Intersects(a->bounds, b->bounds))
    {
        return;
    }

    if (a->IsLeaf() && b->IsLeaf())
    {
        const size_t first = std::min(a->objectIndex, b->objectIndex);
        const size_t second = std::max(a->objectIndex, b->objectIndex);
        results.push_back({first, second});
        return;
    }

    if (a->IsLeaf())
    {
        CollectOverlaps(a, b->left.get(), results);
        CollectOverlaps(a, b->right.get(), results);
    }
    else if (b->IsLeaf())
    {
        CollectOverlaps(a->left.get(), b, results);
        CollectOverlaps(a->right.get(), b, results);
    }
    else
    {
        CollectOverlaps(a->left.get(), b->left.get(), results);
        CollectOverlaps(a->left.get(), b->right.get(), results);
        CollectOverlaps(a->right.get(), b->left.get(), results);
        CollectOverlaps(a->right.get(), b->right.get(), results);
    }
}

void KdTree::SelfCollide(const Node* node, std::vector<std::pair<size_t, size_t>>& results)
{
    if (node == nullptr || node->IsLeaf())
    {
        return;
    }
    SelfCollide(node->left.get(), results);
    SelfCollide(node->right.get(), results);
    CollectOverlaps(node->left.get(), node->right.get(), results);
}

std::vector<std::pair<size_t, size_t>> KdTree::FindOverlappingPairs() const
{
    std::vector<std::pair<size_t, size_t>> results;
    SelfCollide(root_.get(), results);
    return results;
}
