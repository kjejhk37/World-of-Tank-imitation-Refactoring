#pragma once

// Author: Claude
// Description: 2차원 벡터. 텍스처 좌표(UV)를 표현하는 용도로 주로 쓰인다.
// Input: 생성자 - x, y (기본값 0,0)
// Output: (해당 없음 - 데이터 구조체)
// Notes: Vec3와 마찬가지로 프로젝트의 다른 모듈을 참조하지 않는다.
// Date: 2026-07-23
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y)
    {
    }

    constexpr bool operator==(const Vec2& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    constexpr bool operator!=(const Vec2& rhs) const
    {
        return !(*this == rhs);
    }
};
