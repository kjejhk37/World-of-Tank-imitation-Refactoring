#include "projects/engine/InstanceUpdateWorker.h"

#include <chrono>
#include <cmath>

#include "platform/math/Quaternion.h"
#include "platform/math/Vec3.h"

namespace
{
    constexpr float kGridSpacing = 2.5f;
    constexpr float kBobAmplitude = 0.5f;
    constexpr float kDegreesPerSecond = 45.0f;
    constexpr int kTickIntervalMilliseconds = 16;
}

InstanceUpdateWorker::InstanceUpdateWorker(std::size_t instanceCount) : m_instanceCount(instanceCount)
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
    const Clock::time_point startTime = Clock::now();
    const int gridSize = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(m_instanceCount))));

    while (!m_stopRequested.load(std::memory_order_relaxed))
    {
        const float elapsedSeconds = std::chrono::duration<float>(Clock::now() - startTime).count();

        InstanceSnapshot& snapshot = m_publisher.AcquireWriteSlot();
        snapshot.worldMatrices.resize(m_instanceCount);
        for (std::size_t i = 0; i < m_instanceCount; ++i)
        {
            const int row = gridSize == 0 ? 0 : static_cast<int>(i) / gridSize;
            const int col = gridSize == 0 ? 0 : static_cast<int>(i) % gridSize;

            const Vec3 position(static_cast<float>(col) * kGridSpacing,
                                 std::sin(elapsedSeconds + static_cast<float>(i)) * kBobAmplitude,
                                 static_cast<float>(row) * kGridSpacing);
            const Quaternion rotation = Quaternion::FromAxisAngle(
                Vec3(0.0f, 1.0f, 0.0f), elapsedSeconds * kDegreesPerSecond + static_cast<float>(i) * 10.0f);

            snapshot.worldMatrices[i] = Matrix4x4::FromTRS(position, rotation, Vec3::One());
        }
        m_publisher.Publish();

        std::this_thread::sleep_for(std::chrono::milliseconds(kTickIntervalMilliseconds));
    }
}
