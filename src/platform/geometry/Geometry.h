#pragma once

#include <variant>

#include "platform/geometry/AABB.h"
#include "platform/geometry/Capsule.h"
#include "platform/geometry/Cylinder.h"
#include "platform/geometry/Mesh.h"
#include "platform/geometry/OBB.h"
#include "platform/geometry/Sphere.h"
#include "platform/math/Vec3.h"

// Author: Claude
// Description: 6종 도형(Sphere/AABB/OBB/Capsule/Cylinder/Mesh)을 하나의 타입으로 통일해 다루기 위한
//              std::variant 별칭과, 그 위에서 동작하는 자유 함수(Support/GetBounds) 선언.
//              충돌 감지 알고리즘(GJK/EPA, BVH, R-Tree 등)이 도형 종류를 몰라도 되게 하는 공용 입력 타입.
// Input: Support(geometry, direction) - 방향 벡터(정규화 불필요) / GetBounds(geometry) - 없음
// Output: Support - 그 방향으로 가장 먼 도형 위의 점(월드 좌표) / GetBounds - 도형을 포함하는 AABB
// Notes: 이 파일과 Geometry.cpp 밖에서는 std::visit/std::get을 직접 호출하지 않는다 — 다른 모든 모듈은
//        반드시 Support()/GetBounds() 자유 함수로만 Geometry에 접근한다(접근 퍼사드 규칙). 이렇게 해두면
//        추후 메모리 문제로 가상 인터페이스 기반 설계로 전환하더라도 비용이 이 파일 + geometry를 컬렉션으로
//        저장하는 소수 지점으로 국한된다 — docs/brainstorming/충돌감지_알고리즘_20260720_2119.md 참고.
// Date: 2026-07-20
using Geometry = std::variant<Sphere, AABB, OBB, Capsule, Cylinder, Mesh>;

Vec3 Support(const Geometry& geometry, const Vec3& direction);
AABB GetBounds(const Geometry& geometry);
