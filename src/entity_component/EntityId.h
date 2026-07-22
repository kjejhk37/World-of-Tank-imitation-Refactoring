#pragma once

#include <cstdint>

// Author: Claude
// Description: Entity를 식별하는 opaque ID 타입. enum class로 감싸 ComponentId와 타입 레벨에서 구분한다.
// Input: (해당 없음 — 값 타입)
// Output: (해당 없음 — 값 타입)
// Notes: 진짜 UUID가 아니라 UniqueIdGenerator<EntityId>가 발급하는 프로세스 내 유일 정수다.
//        런타임 오버헤드 없이 EntityId/ComponentId를 실수로 바꿔 쓰는 걸 컴파일 타임에 막기 위한 강한 타입이다.
//        C++14부터 std::hash가 모든 enum 타입에 특수화되어 있어 별도 해시 함수 없이 unordered_map 키로 쓸 수 있다.
// Date: 2026-07-22
enum class EntityId : std::uint64_t
{
};
