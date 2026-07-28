#pragma once

#include <array>
#include <atomic>
#include <cstdint>

#include "engine/IFrameDataPublisher.h"

// Author: Claude
// Description: IFrameDataPublisher<T>의 락-프리 트리플 버퍼 구현. 슬롯을 3개(write/read/spare) 두고,
//              producer(AcquireWriteSlot/Publish)와 consumer(AcquireReadSnapshot)는 각자 전용
//              write/read 슬롯만 건드리며, 공유되는 건 "지금 아무도 안 쓰는 spare 슬롯"의 인덱스뿐이다.
//              Publish()/swap-in은 이 spare 인덱스를 원자적 exchange로 주고받아 write 슬롯과 read
//              슬롯이 항상 서로 다른 두 슬롯이 되도록 보장한다 - 그 결과 producer가 consumer의 read
//              슬롯을 동시에 덮어쓰는 torn read가 구조적으로 발생할 수 없다(비-POD 페이로드에도 안전).
//              공개 클래스 이름은 이전 설계(슬롯 2개 + 원자적 인덱스 스왑)와의 연속성을 위해
//              DoubleBufferPublisher로 유지하되, 내부 저장소는 3슬롯이다.
// Input: (해당 없음 - 기본 생성자가 T의 기본 생성자로 세 슬롯을 초기화)
// Output: (해당 없음 - IFrameDataPublisher<T> 구현)
// Notes: AcquireWriteSlot()이 반환하는 참조에 값을 전부 채운 뒤에만 Publish()를 호출해야 한다 -
//        쓰기 도중 Publish()를 호출하면 그 시점까지 쓰인 값만 반영된 스냅샷이 발행된다(계약 위반,
//        torn read는 아니지만 여전히 데이터 정합성 위반). 단일 producer/단일 consumer(SPSC) 전제 -
//        여러 스레드가 동시에 AcquireWriteSlot()을 호출하거나, 여러 스레드가 동시에
//        AcquireReadSnapshot()을 호출하는 건 지원하지 않는다(멀티 producer/consumer가 필요해지면
//        별도 정책 클래스로 확장). AcquireReadSnapshot()은 논리적으로 "읽기 전용"이지만 내부적으로
//        swap-in 여부를 판단해 m_readIndex/m_spareState를 갱신하므로 멤버가 mutable이다.
// Date: 2026-07-28
template <typename T>
class DoubleBufferPublisher final : public IFrameDataPublisher<T>
{
public:
    DoubleBufferPublisher()
        : m_slots{}
        , m_writeIndex(0)
        , m_readIndex(1)
        , m_spareState(EncodeState(2, false))
    {
    }

    T& AcquireWriteSlot() override
    {
        return m_slots[m_writeIndex];
    }

    void Publish() override
    {
        const std::uint32_t published = EncodeState(m_writeIndex, true);
        const std::uint32_t previousSpare = m_spareState.exchange(published, std::memory_order_acq_rel);
        m_writeIndex = DecodeIndex(previousSpare);
    }

    const T& AcquireReadSnapshot() const override
    {
        const std::uint32_t currentSpare = m_spareState.load(std::memory_order_acquire);
        if (DecodeIsNew(currentSpare))
        {
            const std::uint32_t handedBack = EncodeState(m_readIndex, false);
            const std::uint32_t previousSpare = m_spareState.exchange(handedBack, std::memory_order_acq_rel);
            m_readIndex = DecodeIndex(previousSpare);
        }
        return m_slots[m_readIndex];
    }

private:
    static constexpr std::uint32_t kIndexMask = 0x3u;
    static constexpr std::uint32_t kNewFlag = 0x4u;

    static std::uint32_t EncodeState(int index, bool isNew)
    {
        return static_cast<std::uint32_t>(index) | (isNew ? kNewFlag : 0u);
    }

    static int DecodeIndex(std::uint32_t state)
    {
        return static_cast<int>(state & kIndexMask);
    }

    static bool DecodeIsNew(std::uint32_t state)
    {
        return (state & kNewFlag) != 0u;
    }

    mutable std::array<T, 3> m_slots;
    int m_writeIndex;
    mutable int m_readIndex;
    mutable std::atomic<std::uint32_t> m_spareState;
};
