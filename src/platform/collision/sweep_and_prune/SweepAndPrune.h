#pragma once

#include <utility>
#include <vector>

#include "platform/geometry/Geometry.h"

// Author: Claude
// Description: Sweep and Prune(축 정렬 스윕) broad-phase. 여러 Geometry 중 AABB가 겹치는(실제 충돌
//              검사를 해볼 만한) 후보 쌍만 골라낸다.
// Input: FindOverlappingPairs(objects) - Geometry 목록
// Output: objects 내 index 쌍(first < second) 목록 — AABB가 겹치는 쌍만 포함
// Notes: Geometry.h의 GetBounds() 자유 함수만 사용한다(접근 퍼사드 규칙). 항상 x축 하나로만 정렬해
//        스윕한다 — 분산이 가장 큰 축을 고르는 최적화는 이번 사이클 범위 밖이다.
// Date: 2026-07-20
std::vector<std::pair<size_t, size_t>> FindOverlappingPairs(const std::vector<Geometry>& objects);
