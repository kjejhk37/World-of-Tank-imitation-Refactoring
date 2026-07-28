#pragma once

#include <atomic>
#include <cstddef>
#include <thread>

#include "engine/DoubleBufferPublisher.h"
#include "engine/IFrameDataPublisher.h"
#include "engine/InstanceSnapshot.h"

// Author: Claude
// Description: 실제 게임플레이 데이터(물리/애니메이션)가 아직 없는 이번 사이클의 데모용 Engine
//              producer. N개 인스턴스를 격자로 배치하고 시간에 따라 회전/상하 이동시키는 합성
//              데이터를 자기 속도(고정 sleep 간격)로 계속 계산해 DoubleBufferPublisher에 커밋한다.
// Input: 생성자 - instanceCount(인스턴스 개수, 기본값 kDefaultInstanceCount)
// Output: GetPublisher() - Render(consumer)가 스냅샷을 읽어갈 IFrameDataPublisher<InstanceSnapshot>
// Notes: Start()/Stop()은 워커 스레드의 생애주기 전체에 걸쳐 각각 1회만 호출되는 것을 전제로 한다 -
//        Stop()은 매 프레임이 아니라 셧다운 경로에서만 join()한다. 소멸자가 방어적으로 Stop()을
//        호출하므로, 호출자가 Stop()을 깜빡해도 스레드가 누수/디택되지 않는다(Stop()은 실행 중이
//        아니면 아무 일도 하지 않아 중복 호출에 안전하다). 이후 물리/애니메이션 시스템이 생기면
//        이 클래스를 그 시스템으로 교체한다 - main.cpp/IRenderer는 InstanceSnapshot만 알면 되므로
//        교체 비용은 이 클래스 내부에 국한된다.
// Date: 2026-07-28
class InstanceUpdateWorker
{
public:
    static constexpr std::size_t kDefaultInstanceCount = 16;

    explicit InstanceUpdateWorker(std::size_t instanceCount = kDefaultInstanceCount);
    ~InstanceUpdateWorker();

    void Start();
    void Stop();

    const IFrameDataPublisher<InstanceSnapshot>& GetPublisher() const;

private:
    void Run();

    std::size_t m_instanceCount;
    DoubleBufferPublisher<InstanceSnapshot> m_publisher;
    std::thread m_thread;
    std::atomic<bool> m_stopRequested{false};
    std::atomic<bool> m_running{false};
};
