#include "platform/geometry/Geometry.h"

#include <algorithm>

// Author: Claude
// Description: Geometry.h에 선언된 Support()/GetBounds()의 std::visit 기반 구현.
// Input: (Geometry.h 참고)
// Output: (Geometry.h 참고)
// Notes: Overload는 여러 람다를 하나의 오버로드 집합으로 묶어 std::visit에 넘기기 위한 표준 관용구다 —
//        variant에 새 도형 타입이 추가되는데 대응하는 람다가 없으면 오버로드 결정 실패로 컴파일 에러가 나서,
//        가상 함수의 순수 가상 함수 누락 강제와 동등한 안전성을 준다.
//        Mesh의 Support/GetBounds는 정점을 O(n) 순회한다 — 가속 구조(BVH 등)는 이후 로드맵 항목에서 다룬다.
//        빈 Mesh(vertices.empty())는 예외를 던지지 않고 Support→Vec3::Zero(), GetBounds→원점 퇴화 AABB로
//        방어적으로 처리한다(크래시 방지 목적, 의미상 "유효한 경계"를 뜻하지 않음).
// Date: 2026-07-20

namespace
{
    template <class... Ts>
    struct Overload : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

    AABB EndpointsAABB(const Vec3& a, const Vec3& b, float radius)
    {
        const Vec3 radiusVec(radius, radius, radius);
        const Vec3 lower(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
        const Vec3 upper(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
        return AABB(lower - radiusVec, upper + radiusVec);
    }
}

Vec3 Support(const Geometry& geometry, const Vec3& direction)
{
    return std::visit(
        Overload{
            [&](const Sphere& sphere) { return sphere.center + direction.Normalized() * sphere.radius; },
            [&](const AABB& box) {
                return Vec3(direction.x >= 0.0f ? box.max.x : box.min.x, direction.y >= 0.0f ? box.max.y : box.min.y,
                            direction.z >= 0.0f ? box.max.z : box.min.z);
            },
            [&](const OBB& box) {
                Vec3 point = box.transform.position;
                for (int i = 0; i < 3; ++i)
                {
                    const Vec3 axis = box.GetAxis(i);
                    const float extent = box.halfExtents[i];
                    const float sign = Dot(direction, axis) >= 0.0f ? 1.0f : -1.0f;
                    point += axis * (sign * extent);
                }
                return point;
            },
            [&](const Capsule& capsule) {
                const bool towardB = Dot(direction, capsule.pointB - capsule.pointA) >= 0.0f;
                const Vec3& cap = towardB ? capsule.pointB : capsule.pointA;
                return cap + direction.Normalized() * capsule.radius;
            },
            [&](const Cylinder& cylinder) {
                const Vec3 axis = (cylinder.pointB - cylinder.pointA).Normalized();
                const bool towardB = Dot(direction, axis) >= 0.0f;
                const Vec3& cap = towardB ? cylinder.pointB : cylinder.pointA;
                const Vec3 radial = direction - axis * Dot(direction, axis);
                return cap + radial.Normalized() * cylinder.radius;
            },
            [&](const Mesh& mesh) {
                if (mesh.vertices.empty())
                {
                    return Vec3::Zero();
                }
                Vec3 best = mesh.vertices[0];
                float bestDot = Dot(direction, best);
                for (size_t i = 1; i < mesh.vertices.size(); ++i)
                {
                    const float d = Dot(direction, mesh.vertices[i]);
                    if (d > bestDot)
                    {
                        bestDot = d;
                        best = mesh.vertices[i];
                    }
                }
                return best;
            },
        },
        geometry);
}

AABB GetBounds(const Geometry& geometry)
{
    return std::visit(
        Overload{
            [&](const Sphere& sphere) {
                const Vec3 radiusVec(sphere.radius, sphere.radius, sphere.radius);
                return AABB(sphere.center - radiusVec, sphere.center + radiusVec);
            },
            [&](const AABB& box) { return box; },
            [&](const OBB& box) {
                Vec3 extent = Vec3::Zero();
                for (int i = 0; i < 3; ++i)
                {
                    const Vec3 axis = box.GetAxis(i);
                    const float he = box.halfExtents[i];
                    extent.x += std::abs(axis.x) * he;
                    extent.y += std::abs(axis.y) * he;
                    extent.z += std::abs(axis.z) * he;
                }
                return AABB(box.transform.position - extent, box.transform.position + extent);
            },
            [&](const Capsule& capsule) { return EndpointsAABB(capsule.pointA, capsule.pointB, capsule.radius); },
            [&](const Cylinder& cylinder) { return EndpointsAABB(cylinder.pointA, cylinder.pointB, cylinder.radius); },
            [&](const Mesh& mesh) {
                if (mesh.vertices.empty())
                {
                    return AABB(Vec3::Zero(), Vec3::Zero());
                }
                Vec3 lower = mesh.vertices[0];
                Vec3 upper = mesh.vertices[0];
                for (size_t i = 1; i < mesh.vertices.size(); ++i)
                {
                    const Vec3& v = mesh.vertices[i];
                    lower = Vec3(std::min(lower.x, v.x), std::min(lower.y, v.y), std::min(lower.z, v.z));
                    upper = Vec3(std::max(upper.x, v.x), std::max(upper.y, v.y), std::max(upper.z, v.z));
                }
                return AABB(lower, upper);
            },
        },
        geometry);
}
