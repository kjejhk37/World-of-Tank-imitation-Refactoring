#include "projects/engine/InstanceUpdateWorker.h"

#include <chrono>

#include "projects/game/GameManager.h"
#include "projects/game/SceneSnapshotBuilder.h"

namespace
{
    constexpr int kTickIntervalMilliseconds = 16;
}

InstanceUpdateWorker::InstanceUpdateWorker()
{
}

InstanceUpdateWorker::~InstanceUpdateWorker()
{
    Stop();
}

void InstanceUpdateWorker::Start()
{
    if (m_running.load(std::memory_order_relaxed))
    {
        return;
    }

    m_stopRequested.store(false, std::memory_order_relaxed);
    m_running.store(true, std::memory_order_relaxed);
    m_thread = std::thread(&InstanceUpdateWorker::Run, this);
}

void InstanceUpdateWorker::Stop()
{
    if (!m_running.load(std::memory_order_relaxed))
    {
        return;
    }

    m_stopRequested.store(true, std::memory_order_relaxed);
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    m_running.store(false, std::memory_order_relaxed);
}

const IFrameDataPublisher<InstanceSnapshot>& InstanceUpdateWorker::GetPublisher() const
{
    return m_publisher;
}

void InstanceUpdateWorker::Run()
{
    using Clock = std::chrono::steady_clock;

    GameManager gameManager;
    Clock::time_point lastTick = Clock::now();

    while (!m_stopRequested.load(std::memory_order_relaxed))
    {
        const Clock::time_point now = Clock::now();
        const float deltaTime = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        gameManager.Update(deltaTime);

        InstanceSnapshot& snapshot = m_publisher.AcquireWriteSlot();
        SceneSnapshotBuilder::Build(gameManager.GetSceneManager(), snapshot);
        m_publisher.Publish();

        std::this_thread::sleep_for(std::chrono::milliseconds(kTickIntervalMilliseconds));
    }
}
