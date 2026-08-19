#pragma once

#include <atomic>
#include <thread>

#include "platform/engine/DoubleBufferPublisher.h"
#include "platform/engine/IFrameDataPublisher.h"
#include "graphics/renderer/InstanceSnapshot.h"

// Author: Claude
// Description: engine 스레드 자체(수명주기·틱 루프)를 담당하는 Engine producer. 매 틱 GameManager::Update()로
//              씬 로직을 갱신하고, SceneSnapshotBuilder로 그 결과를 InstanceSnapshot으로 변환해
//              DoubleBufferPublisher에 커밋한다. 실제 게임 로직(IntroScene 등)은 이 클래스가 몰라도
//              되도록 GameManager 뒤에 캡슐화되어 있다.
// Input: (해당 없음)
// Output: GetPublisher() - Render(consumer)가 스냅샷을 읽어갈 IFrameDataPublisher<InstanceSnapshot>
// Notes: Start()/Stop()은 워커 스레드의 생애주기 전체에 걸쳐 각각 1회만 호출되는 것을 전제로 한다 -
//        Stop()은 매 프레임이 아니라 셧다운 경로에서만 join()한다. 소멸자가 방어적으로 Stop()을
//        호출하므로, 호출자가 Stop()을 깜빡해도 스레드가 누수/디택되지 않는다(Stop()은 실행 중이
//        아니면 아무 일도 하지 않아 중복 호출에 안전하다). GameManager는 Run() 안의 지역 변수로
//        스레드 수명 동안 1회만 생성된다 - main.cpp/IRenderer는 여전히 InstanceSnapshot만 알면 되므로
//        이후 실제 물리/애니메이션이 늘어나도 교체 비용은 GameManager/Scene 쪽에 국한된다.
// Date: 2026-08-19
class InstanceUpdateWorker
{
public:
    InstanceUpdateWorker();
    ~InstanceUpdateWorker();

    void Start();
    void Stop();

    const IFrameDataPublisher<InstanceSnapshot>& GetPublisher() const;

private:
    void Run();

    DoubleBufferPublisher<InstanceSnapshot> m_publisher;
    std::thread m_thread;
    std::atomic<bool> m_stopRequested{false};
    std::atomic<bool> m_running{false};
};
