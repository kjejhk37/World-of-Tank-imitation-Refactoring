#include "collision/sweep_and_prune/SweepAndPrune.h"

#include <algorithm>

#include "geometry/Intersections.h"

// Author: Claude
// Description: FindOverlappingPairs()의 구현 — min.x 기준 정렬 후 정렬 순서를 이용해 조기 가지치기한다.
// Input/Output: (SweepAndPrune.h 참고)
// Notes: 실제 겹침 판정은 geometry/Intersections.h의 기존 Intersects(AABB,AABB)를 재사용한다 — 이
//        모듈은 "정렬 + 가지치기" 책임만 진다.
// Date: 2026-07-20

namespace
{
    struct Entry
    {
        size_t originalIndex;
        AABB bounds;
    };
}

std::vector<std::pair<size_t, size_t>> FindOverlappingPairs(const std::vector<Geometry>& objects)
{
    std::vector<Entry> entries;
    entries.reserve(objects.size());
    for (size_t i = 0; i < objects.size(); ++i)
    {
        entries.push_back(Entry{i, GetBounds(objects[i])});
    }

    std::sort(entries.begin(), entries.end(),
              [](const Entry& lhs, const Entry& rhs) { return lhs.bounds.min.x < rhs.bounds.min.x; });

    std::vector<std::pair<size_t, size_t>> pairs;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        for (size_t j = i + 1; j < entries.size(); ++j)
        {
            if (entries[j].bounds.min.x > entries[i].bounds.max.x)
            {
                break;
            }
            if (Intersects(entries[i].bounds, entries[j].bounds))
            {
                const size_t first = std::min(entries[i].originalIndex, entries[j].originalIndex);
                const size_t second = std::max(entries[i].originalIndex, entries[j].originalIndex);
                pairs.push_back({first, second});
            }
        }
    }
    return pairs;
}
