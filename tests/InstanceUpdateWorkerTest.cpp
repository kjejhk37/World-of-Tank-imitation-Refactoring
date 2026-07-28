#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "engine/InstanceUpdateWorker.h"

TEST(InstanceUpdateWorkerTest, StartPopulatesSnapshotWithRequestedInstanceCount)
{
    constexpr std::size_t kInstanceCount = 4;
    InstanceUpdateWorker worker(kInstanceCount);

    EXPECT_TRUE(worker.GetPublisher().AcquireReadSnapshot().worldMatrices.empty());

    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const InstanceSnapshot& snapshot = worker.GetPublisher().AcquireReadSnapshot();
    EXPECT_EQ(snapshot.worldMatrices.size(), kInstanceCount);

    worker.Stop();
}

TEST(InstanceUpdateWorkerTest, StopReturnsWithoutHangingOrCrashing)
{
    InstanceUpdateWorker worker(4);

    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    worker.Stop();

    // 반복 호출이 안전한지(중복 join 없이 조용히 무시되는지) 확인.
    worker.Stop();

    SUCCEED();
}

TEST(InstanceUpdateWorkerTest, SnapshotKeepsChangingOverTime)
{
    InstanceUpdateWorker worker(4);
    worker.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const Matrix4x4 first = worker.GetPublisher().AcquireReadSnapshot().worldMatrices.at(0);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    const Matrix4x4 second = worker.GetPublisher().AcquireReadSnapshot().worldMatrices.at(0);

    worker.Stop();

    bool anyComponentDiffers = false;
    for (int row = 0; row < 4 && !anyComponentDiffers; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            if (first.m[row][col] != second.m[row][col])
            {
                anyComponentDiffers = true;
                break;
            }
        }
    }
    EXPECT_TRUE(anyComponentDiffers);
}
