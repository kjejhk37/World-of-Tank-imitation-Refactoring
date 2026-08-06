#pragma once

#include <cstdint>

// Author: Claude
// Description: Component를 식별하는 opaque ID 타입. enum class로 감싸 EntityId와 타입 레벨에서 구분한다.
// Input: (해당 없음 — 값 타입)
// Output: (해당 없음 — 값 타입)
// Notes: 진짜 UUID가 아니라 UniqueIdGenerator<ComponentId>가 발급하는 프로세스 내 유일 정수다.
//        한 Entity가 같은 타입 Component를 여러 개 가질 수 있어(예: 신체 부위별 Collider), 저장소 키로
//        타입이 아니라 이 ComponentId를 쓴다 (docs/brainstorming/Component_패턴_기본구조_20260722_2217.md 참고).
// Date: 2026-07-22
enum class ComponentId : std::uint64_t
{
};
