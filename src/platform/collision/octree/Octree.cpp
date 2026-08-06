#include "platform/collision/octree/Octree.h"

#include <algorithm>

#include "platform/geometry/Intersections.h"

// Author: Claude
// Description: Octree 멤버 함수 구현 — 적응형 8분할 빌드 + self-collide 쿼리.
// Input/Output: (Octree.h 참고)
// Date: 2026-07-20

Octree::Octree(const std::vector<Geometry>& objects, const AABB& worldBounds, int maxDepth)
{
    root_ = std::make_unique<Node>();
    root_->bounds = worldBounds;
    root_->depth = 0;

    objectBounds_.reserve(objects.size());
    for (const Geometry& object : objects)
    {
        objectBounds_.push_back(GetBounds(object));
    }

    for (size_t i = 0; i < objectBounds_.size(); ++i)
    {
        Insert(root_.get(), i, objectBounds_[i], maxDepth);
    }
}

void Octree::Subdivide(Node* node)
{
    const Vec3 center = node->bounds.Center();
    for (int i = 0; i < 8; ++i)
    {
        Vec3 childMin;
        Vec3 childMax;
        childMin.x = (i & 1) ? center.x : node->bounds.min.x;
        childMax.x = (i & 1) ? node->bounds.max.x : center.x;
        childMin.y = (i & 2) ? center.y : node->bounds.min.y;
        childMax.y = (i & 2) ? node->bounds.max.y : center.y;
        childMin.z = (i & 4) ? center.z : node->bounds.min.z;
        childMax.z = (i & 4) ? node->bounds.max.z : center.z;

        auto child = std::make_unique<Node>();
        child->bounds = AABB(childMin, childMax);
        child->depth = node->depth + 1;
        node->children[static_cast<size_t>(i)] = std::move(child);
    }
    node->subdivided = true;
}

int Octree::FindContainingChild(const Node* node, const AABB& bounds)
{
    const Vec3 center = node->bounds.Center();
    int index = 0;

    if (bounds.max.x <= center.x)
    {
    }
    else if (bounds.min.x >= center.x)
    {
        index |= 1;
    }
    else
    {
        return -1;
    }

    if (bounds.max.y <= center.y)
    {
    }
    else if (bounds.min.y >= center.y)
    {
        index |= 2;
    }
    else
    {
        return -1;
    }

    if (bounds.max.z <= center.z)
    {
    }
    else if (bounds.min.z >= center.z)
    {
        index |= 4;
    }
    else
    {
        return -1;
    }

    return index;
}

void Octree::Insert(Node* node, size_t objectIndex, const AABB& bounds, int maxDepth)
{
    if (node->depth < maxDepth)
    {
        if (!node->subdivided && node->localObjects.size() >= kMaxLocalBeforeSubdivide)
        {
            Subdivide(node);
        }
        if (node->subdivided)
        {
            const int childIndex = FindContainingChild(node, bounds);
            if (childIndex >= 0)
            {
                Insert(node->children[static_cast<size_t>(childIndex)].get(), objectIndex, bounds, maxDepth);
                return;
            }
        }
    }
    node->localObjects.push_back(objectIndex);
}

void Octree::CheckAgainstSubtree(const std::vector<size_t>& ancestorObjects, const Node* node,
                                  const std::vector<AABB>& bounds, std::vector<std::pair<size_t, size_t>>& results)
{
    for (size_t a : ancestorObjects)
    {
        for (size_t b : node->localObjects)
        {
            if (Intersects(bounds[a], bounds[b]))
            {
                results.push_back({std::min(a, b), std::max(a, b)});
            }
        }
    }
    for (const auto& child : node->children)
    {
        if (child)
        {
            CheckAgainstSubtree(ancestorObjects, child.get(), bounds, results);
        }
    }
}

void Octree::SelfCollide(const Node* node, const std::vector<AABB>& bounds,
                          std::vector<std::pair<size_t, size_t>>& results)
{
    for (size_t i = 0; i < node->localObjects.size(); ++i)
    {
        for (size_t j = i + 1; j < node->localObjects.size(); ++j)
        {
            const size_t a = node->localObjects[i];
            const size_t b = node->localObjects[j];
            if (Intersects(bounds[a], bounds[b]))
            {
                results.push_back({std::min(a, b), std::max(a, b)});
            }
        }
    }

    for (const auto& child : node->children)
    {
        if (child)
        {
            CheckAgainstSubtree(node->localObjects, child.get(), bounds, results);
        }
    }

    for (const auto& child : node->children)
    {
        if (child)
        {
            SelfCollide(child.get(), bounds, results);
        }
    }
}

std::vector<std::pair<size_t, size_t>> Octree::FindOverlappingPairs() const
{
    std::vector<std::pair<size_t, size_t>> results;
    SelfCollide(root_.get(), objectBounds_, results);
    return results;
}
