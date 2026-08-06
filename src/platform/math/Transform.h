#pragma once

#include "platform/math/Matrix4x4.h"
#include "platform/math/Quaternion.h"
#include "platform/math/Vec3.h"

// Author: Claude
// Description: 위치/회전/스케일(TRS)을 하나로 묶는 범용 클래스. OBB를 포함해 향후 게임 오브젝트/카메라 등
//              공간 상의 위치+방향이 필요한 곳에서 재사용할 수 있게 설계했다.
// Input: (해당 없음 - 데이터 구조체, 기본값은 항등 변환)
// Output: ToMatrix() - 이 Transform과 동일한 변환을 나타내는 Matrix4x4 /
//         GetAxisX/Y/Z() - 이 Transform의 회전이 적용된 로컬 X/Y/Z축(월드 공간 방향 벡터)
// Notes: scale은 기본값 (1,1,1) — OBB처럼 크기를 별도 필드(halfExtents)로 갖는 도형은 scale을
//        항상 (1,1,1)로 두고 쓰지 않는다(크기 정보를 두 곳에 중복해서 두지 않기 위함).
// Date: 2026-07-19
struct Transform
{
    Vec3 position;
    Quaternion rotation = Quaternion::Identity();
    Vec3 scale = Vec3::One();

    Matrix4x4 ToMatrix() const
    {
        return Matrix4x4::FromTRS(position, rotation, scale);
    }

    Vec3 GetAxisX() const
    {
        return rotation.Rotate(Vec3(1.0f, 0.0f, 0.0f));
    }

    Vec3 GetAxisY() const
    {
        return rotation.Rotate(Vec3(0.0f, 1.0f, 0.0f));
    }

    Vec3 GetAxisZ() const
    {
        return rotation.Rotate(Vec3(0.0f, 0.0f, 1.0f));
    }
};
