#pragma once

// Author: Claude
// Description: Engine(producer)이 계산한 데이터를 Render(consumer)에게 넘기는 방식을 추상화하는
//              인터페이스. 구체적인 동기화 정책(더블 버퍼링/Fork-Join/큐 등)은 구현체가 결정하고,
//              이 인터페이스에 의존하는 코드(Engine 계산 로직/IRenderer)는 어떤 정책이 쓰이는지
//              몰라도 된다.
// Input: (해당 없음 - 인터페이스)
// Output: (해당 없음 - 인터페이스)
// Notes: AcquireWriteSlot()으로 얻은 참조에 값을 전부 채운 뒤에만 Publish()를 호출해야 한다는 계약을
//        각 구현체가 보장한다 - 이 순서를 어기면 구현체에 따라 torn read가 발생할 수 있다.
// Date: 2026-07-26
template <typename T>
class IFrameDataPublisher
{
public:
    virtual ~IFrameDataPublisher() = default;

    virtual T& AcquireWriteSlot() = 0;
    virtual void Publish() = 0;
    virtual const T& AcquireReadSnapshot() const = 0;
};
