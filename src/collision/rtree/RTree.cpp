#include "collision/rtree/RTree.h"

#include <algorithm>
#include <limits>

#include "geometry/Intersections.h"

// Author: Claude
// Description: RTree 멤버 함수 구현 — Guttman의 고전 R-Tree 삽입/quadratic split + self-collide 쿼리.
// Input/Output: (RTree.h 참고)
// Notes: 리프-리프 최종 판정은 geometry/Intersections.h의 기존 Intersects(AABB,AABB)를 재사용한다.
// Date: 2026-07-20

namespace
{
    float Volume(const AABB& box)
    {
        const Vec3 extents = box.Extents();
        return (2.0f * extents.x) * (2.0f * extents.y) * (2.0f * extents.z);
    }

    AABB Merge(const AABB& a, const AABB& b)
    {
        return AABB(Vec3(std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z)),
                     Vec3(std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z)));
    }
}

RTree::RTree(const std::vector<Geometry>& objects)
{
    for (const Geometry& object : objects)
    {
        Insert(object);
    }
}

void RTree::Insert(const Geometry& object)
{
    Entry entry;
    entry.bounds = GetBounds(object);
    entry.objectIndex = nextIndex_++;
    InsertLeafEntry(std::move(entry));
}

void RTree::InsertLeafEntry(Entry entry)
{
    std::unique_ptr<Node> sibling = InsertRecursive(root_.get(), std::move(entry));
    if (sibling == nullptr)
    {
        return;
    }

    auto newRoot = std::make_unique<Node>();
    newRoot->leaf = false;

    Entry oldRootEntry;
    oldRootEntry.bounds = ComputeBounds(root_->entries);
    oldRootEntry.child = std::move(root_);

    Entry siblingEntry;
    siblingEntry.bounds = ComputeBounds(sibling->entries);
    siblingEntry.child = std::move(sibling);

    newRoot->entries.push_back(std::move(oldRootEntry));
    newRoot->entries.push_back(std::move(siblingEntry));
    root_ = std::move(newRoot);
}

AABB RTree::ComputeBounds(const std::vector<Entry>& entries)
{
    AABB result = entries[0].bounds;
    for (size_t i = 1; i < entries.size(); ++i)
    {
        result = Merge(result, entries[i].bounds);
    }
    return result;
}

size_t RTree::ChooseSubtree(const std::vector<Entry>& entries, const AABB& bounds)
{
    size_t best = 0;
    float bestEnlargement = std::numeric_limits<float>::max();
    float bestVolume = std::numeric_limits<float>::max();
    for (size_t i = 0; i < entries.size(); ++i)
    {
        const float enlargement = Volume(Merge(entries[i].bounds, bounds)) - Volume(entries[i].bounds);
        const float volume = Volume(entries[i].bounds);
        if (enlargement < bestEnlargement || (enlargement == bestEnlargement && volume < bestVolume))
        {
            best = i;
            bestEnlargement = enlargement;
            bestVolume = volume;
        }
    }
    return best;
}

std::unique_ptr<RTree::Node> RTree::InsertRecursive(Node* node, Entry entry)
{
    if (node->leaf)
    {
        node->entries.push_back(std::move(entry));
    }
    else
    {
        const size_t best = ChooseSubtree(node->entries, entry.bounds);
        std::unique_ptr<Node> sibling = InsertRecursive(node->entries[best].child.get(), std::move(entry));
        node->entries[best].bounds = ComputeBounds(node->entries[best].child->entries);

        if (sibling != nullptr)
        {
            Entry siblingEntry;
            siblingEntry.bounds = ComputeBounds(sibling->entries);
            siblingEntry.child = std::move(sibling);
            node->entries.push_back(std::move(siblingEntry));
        }
    }

    if (node->entries.size() > kMaxEntries)
    {
        return SplitNode(node);
    }
    return nullptr;
}

std::unique_ptr<RTree::Node> RTree::SplitNode(Node* node)
{
    std::vector<Entry> source = std::move(node->entries);

    // Quadratic split: 함께 묶었을 때 "낭비"(합친 부피 - 각자 부피의 합)가 가장 큰 두 항목을 시드로 고른다.
    size_t seedA = 0;
    size_t seedB = 1;
    float worstWaste = -1.0f;
    for (size_t i = 0; i < source.size(); ++i)
    {
        for (size_t j = i + 1; j < source.size(); ++j)
        {
            const float waste =
                Volume(Merge(source[i].bounds, source[j].bounds)) - Volume(source[i].bounds) - Volume(source[j].bounds);
            if (waste > worstWaste)
            {
                worstWaste = waste;
                seedA = i;
                seedB = j;
            }
        }
    }

    auto sibling = std::make_unique<Node>();
    sibling->leaf = node->leaf;

    std::vector<Entry> groupA;
    std::vector<Entry> groupB;
    groupA.push_back(std::move(source[seedA]));
    groupB.push_back(std::move(source[seedB]));

    std::vector<Entry> remaining;
    for (size_t i = 0; i < source.size(); ++i)
    {
        if (i != seedA && i != seedB)
        {
            remaining.push_back(std::move(source[i]));
        }
    }

    AABB boundsA = groupA[0].bounds;
    AABB boundsB = groupB[0].bounds;

    for (Entry& entry : remaining)
    {
        // 남은 그룹이 최소 항목 수를 채울 만큼만 남았다면 강제 배정.
        const size_t neededByA = kMinEntries > groupA.size() ? kMinEntries - groupA.size() : 0;
        const size_t neededByB = kMinEntries > groupB.size() ? kMinEntries - groupB.size() : 0;
        if (remaining.size() <= neededByA + neededByB || neededByA >= remaining.size())
        {
            groupA.push_back(std::move(entry));
            boundsA = Merge(boundsA, groupA.back().bounds);
            continue;
        }
        if (neededByB >= remaining.size())
        {
            groupB.push_back(std::move(entry));
            boundsB = Merge(boundsB, groupB.back().bounds);
            continue;
        }

        const float enlargeA = Volume(Merge(boundsA, entry.bounds)) - Volume(boundsA);
        const float enlargeB = Volume(Merge(boundsB, entry.bounds)) - Volume(boundsB);
        if (enlargeA < enlargeB)
        {
            groupA.push_back(std::move(entry));
            boundsA = Merge(boundsA, groupA.back().bounds);
        }
        else
        {
            groupB.push_back(std::move(entry));
            boundsB = Merge(boundsB, groupB.back().bounds);
        }
    }

    node->entries = std::move(groupA);
    sibling->entries = std::move(groupB);
    return sibling;
}

void RTree::CollectOverlaps(const Node* a, const Node* b, std::vector<std::pair<size_t, size_t>>& results)
{
    for (const Entry& ea : a->entries)
    {
        for (const Entry& eb : b->entries)
        {
            if (!Intersects(ea.bounds, eb.bounds))
            {
                continue;
            }
            if (a->leaf)
            {
                const size_t first = std::min(ea.objectIndex, eb.objectIndex);
                const size_t second = std::max(ea.objectIndex, eb.objectIndex);
                results.push_back({first, second});
            }
            else
            {
                CollectOverlaps(ea.child.get(), eb.child.get(), results);
            }
        }
    }
}

void RTree::SelfCollide(const Node* node, std::vector<std::pair<size_t, size_t>>& results)
{
    if (node->leaf)
    {
        for (size_t i = 0; i < node->entries.size(); ++i)
        {
            for (size_t j = i + 1; j < node->entries.size(); ++j)
            {
                if (Intersects(node->entries[i].bounds, node->entries[j].bounds))
                {
                    const size_t first = std::min(node->entries[i].objectIndex, node->entries[j].objectIndex);
                    const size_t second = std::max(node->entries[i].objectIndex, node->entries[j].objectIndex);
                    results.push_back({first, second});
                }
            }
        }
        return;
    }

    for (size_t i = 0; i < node->entries.size(); ++i)
    {
        SelfCollide(node->entries[i].child.get(), results);
        for (size_t j = i + 1; j < node->entries.size(); ++j)
        {
            CollectOverlaps(node->entries[i].child.get(), node->entries[j].child.get(), results);
        }
    }
}

std::vector<std::pair<size_t, size_t>> RTree::FindOverlappingPairs() const
{
    std::vector<std::pair<size_t, size_t>> results;
    SelfCollide(root_.get(), results);
    return results;
}
