#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "projects/engine/InstanceUpdateWorker.h"

TEST(InstanceUpdateWorkerTest, StopReturnsWithoutHangingOrCrashing)
{
    InstanceUpdateWorker worker;

    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    worker.Stop();

    // 반복 호출이 안전한지(중복 join 없이 조용히 무시되는지) 확인.
    worker.Stop();

    SUCCEED();
}

TEST(InstanceUpdateWorkerTest, PublishesSnapshotWithoutCrashingWhenSceneHasNoEntities)
{
    InstanceUpdateWorker worker;

    EXPECT_TRUE(worker.GetPublisher().AcquireReadSnapshot().worldMatrices.empty());

    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(worker.GetPublisher().AcquireReadSnapshot().worldMatrices.empty());

    worker.Stop();
}
