#pragma once

#include "platform/math/Transform.h"
#include "platform/math/Vec3.h"

// Author: Claude
// Description: 방향이 있는 경계 상자(Oriented Bounding Box). 위치+회전은 Transform으로, 크기는
//              별도 halfExtents 필드로 표현한다.
// Input: 생성자 - transform(위치=중심, 회전=방향, scale은 사용하지 않음), halfExtents(각 축 half-size)
// Output: GetAxis(index) - 0/1/2에 대해 이 OBB의 로컬 X/Y/Z축(월드 공간 방향, 정규직교)
// Notes: transform.scale은 OBB 목적으로는 항상 (1,1,1)로 둔다 — 크기 정보(halfExtents)를 두 곳에
//        중복해서 두지 않기 위함.
//        충돌/포함 판정은 이 클래스에 두지 않고 geometry/Intersections.h에 자유 함수로 모은다.
// Date: 2026-07-19
struct OBB
{
    Transform transform;
    Vec3 halfExtents;

    OBB() = default;
    OBB(const Transform& transform, const Vec3& halfExtents) : transform(transform), halfExtents(halfExtents)
    {
    }

    Vec3 GetAxis(int index) const
    {
        if (index == 0)
        {
            return transform.GetAxisX();
        }
        if (index == 1)
        {
            return transform.GetAxisY();
        }
        return transform.GetAxisZ();
    }
};
