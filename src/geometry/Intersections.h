#pragma once

#include <optional>

#include "geometry/AABB.h"
#include "geometry/Capsule.h"
#include "geometry/Cylinder.h"
#include "geometry/OBB.h"
#include "geometry/Sphere.h"
#include "math/Vec3.h"

// Author: Claude
// Description: 도형 쌍별 포함/충돌 판정 함수 모음(자유 함수). 도형 클래스 자체는 데이터만 갖고,
//              쌍마다 어느 클래스가 상대를 알아야 하는지의 문제(순환 의존)를 피하기 위해 여기 한 곳에
//              모은다.
// Input: (각 함수 시그니처 참고)
// Output: (각 함수 시그니처 참고 — bool 또는 std::optional<AABB>)
// Notes: 이번 사이클은 쌍별 특수 함수로 직접 구현한다(GJK/BVH/FCL 같은 범용 알고리즘/외부 라이브러리는
//        다음 사이클 대상). 경계 비교는 MathConstants::kEpsilon을 일관되게 사용한다.
//        Cylinder가 낀 일부 쌍(Cylinder-Cylinder, Capsule-Cylinder, Cylinder-AABB, Cylinder-OBB)은
//        일반적인 방향에서 닫힌 형태 해가 없어 이번 사이클 범위에서 제외([DEFERRED], 전략 문서 참고).
// Date: 2026-07-19

bool Contains(const AABB& box, const Vec3& point);
bool Contains(const OBB& box, const Vec3& point);

bool Intersects(const AABB& a, const AABB& b);
std::optional<AABB> IntersectionRegion(const AABB& a, const AABB& b);

bool Intersects(const Sphere& a, const Sphere& b);

bool Intersects(const Sphere& sphere, const AABB& box);
bool Intersects(const AABB& box, const Sphere& sphere);

bool Intersects(const Sphere& sphere, const OBB& box);
bool Intersects(const OBB& box, const Sphere& sphere);

bool Intersects(const AABB& a, const OBB& b);
bool Intersects(const OBB& a, const AABB& b);

bool Intersects(const OBB& a, const OBB& b);

bool Intersects(const Sphere& sphere, const Capsule& capsule);
bool Intersects(const Capsule& capsule, const Sphere& sphere);

bool Intersects(const Capsule& a, const Capsule& b);

// --- Tier 2 (전략 문서 참고 — 막히면 단순화/제외 허용) ---

bool Intersects(const Capsule& capsule, const AABB& box);
bool Intersects(const AABB& box, const Capsule& capsule);

bool Intersects(const Capsule& capsule, const OBB& box);
bool Intersects(const OBB& box, const Capsule& capsule);

bool Intersects(const Sphere& sphere, const Cylinder& cylinder);
bool Intersects(const Cylinder& cylinder, const Sphere& sphere);
