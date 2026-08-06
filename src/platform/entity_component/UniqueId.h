#pragma once

#include <atomic>
#include <cstdint>

// Author: Claude
// Description: IdType(예: EntityId, ComponentId)별로 독립된 원자적 카운터에서 유일 정수 값을 발급하는 템플릿.
// Input: (해당 없음 — Next() 호출 시점의 내부 카운터 상태)
// Output: Next() — 호출마다 1씩 증가하는 유일한 IdType 값 (1부터 시작, 0은 "미할당"을 뜻하는 값으로 예약)
// Notes: IdType이 다르면(예: EntityId vs ComponentId) 템플릿 인스턴스화가 갈라지므로 카운터도 서로 독립적이다.
//        진짜 RFC 4122 UUID가 아니라 프로세스 내에서만 유일하면 되는 값으로 충분하다는 브레인스토밍 결론에 따른 설계다
//        (docs/brainstorming/Component_패턴_기본구조_20260722_2217.md 참고). 이 발급 로직만 std::atomic으로
//        스레드 안전하며, 이를 사용하는 Entity/EntityManager 컨테이너 자체는 단일 스레드 접근을 전제로 한다.
// Date: 2026-07-22
template <typename IdType>
class UniqueIdGenerator
{
public:
    static IdType Next()
    {
        static std::atomic<std::uint64_t> counter{1};
        return static_cast<IdType>(counter.fetch_add(1, std::memory_order_relaxed));
    }
};
