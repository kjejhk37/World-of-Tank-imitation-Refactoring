#pragma once

#include "math/Quaternion.h"
#include "math/Vec3.h"

// Author: Claude
// Description: 4x4 동차좌표 행렬. 이동/회전/스케일(TRS) 합성, 점/방향 변환, 아핀 전용 역행렬을 제공한다.
// Input: FromTRS - position(이동), rotation(회전, Quaternion), scale(스케일)
// Output: (해당 없음 - 데이터 구조체 + 연산)
// Notes: **row-major 저장 + row-vector 관례**를 쓴다 — 점 p에 대해 p' = p * M (p는 1x4 행벡터).
//        이 관례에서 이동 성분은 마지막 "행"(m[3][0..2])에 저장된다(OpenGL/열벡터 관례와 반대,
//        DirectX/HLSL 기본 관례와 일치 — 이 프로젝트가 최종적으로 DirectX 파이프라인에 값을 넘기게 될
//        것을 감안한 선택).
//        InverseAffine()은 완전한 일반 4x4 역행렬이 아니라, Transform이 만드는 것과 같은 아핀 행렬
//        (마지막 열이 (0,0,0,1))만을 대상으로 한다 — 상위 3x3을 코팩터(여인수) 공식으로 역변환하고
//        이동 성분을 재계산한다. 상위 3x3이 특이(역행렬 없음)이면 안전하게 Identity()를 반환한다.
// Date: 2026-07-19
struct Matrix4x4
{
    // m[row][col]
    float m[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

    static Matrix4x4 Identity()
    {
        return Matrix4x4();
    }

    static Matrix4x4 FromTRS(const Vec3& position, const Quaternion& rotation, const Vec3& scale);

    Matrix4x4 operator*(const Matrix4x4& rhs) const;

    Vec3 TransformPoint(const Vec3& point) const;
    Vec3 TransformDirection(const Vec3& direction) const;
    Matrix4x4 Transpose() const;
    Matrix4x4 InverseAffine() const;
};
