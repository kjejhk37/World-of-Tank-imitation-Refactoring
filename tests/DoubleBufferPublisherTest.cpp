#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "platform/engine/DoubleBufferPublisher.h"

namespace
{
    struct PairValue
    {
        int a = 0;
        int b = 0;
    };
}

TEST(DoubleBufferPublisherTest, ManualModeReflectsPublishedValueOnSingleThread)
{
    DoubleBufferPublisher<PairValue> publisher;

    EXPECT_EQ(publisher.AcquireReadSnapshot().a, 0);
    EXPECT_EQ(publisher.AcquireReadSnapshot().b, 0);

    PairValue& writeSlot = publisher.AcquireWriteSlot();
    writeSlot.a = 7;
    writeSlot.b = 14;
    publisher.Publish();

    const PairValue& snapshot = publisher.AcquireReadSnapshot();
    EXPECT_EQ(snapshot.a, 7);
    EXPECT_EQ(snapshot.b, 14);
}

TEST(DoubleBufferPublisherTest, ManualModeReflectsMultiplePublishesInOrder)
{
    DoubleBufferPublisher<PairValue> publisher;

    for (int i = 1; i <= 5; ++i)
    {
        PairValue& writeSlot = publisher.AcquireWriteSlot();
        writeSlot.a = i;
        writeSlot.b = i * 2;
        publisher.Publish();

        const PairValue& snapshot = publisher.AcquireReadSnapshot();
        EXPECT_EQ(snapshot.a, i);
        EXPECT_EQ(snapshot.b, i * 2);
    }
}

TEST(DoubleBufferPublisherTest, ConcurrentProducerConsumerNeverObservesTornRead)
{
    DoubleBufferPublisher<PairValue> publisher;
    std::atomic<bool> stopRequested{false};
    std::atomic<bool> tornReadDetected{false};
    std::atomic<std::uint64_t> readCount{0};

    std::thread producer([&publisher, &stopRequested]() {
        int counter = 0;
        while (!stopRequested.load(std::memory_order_relaxed))
        {
            ++counter;
            PairValue& writeSlot = publisher.AcquireWriteSlot();
            writeSlot.a = counter;
            writeSlot.b = counter * 2;
            publisher.Publish();
        }
    });

    std::thread consumer([&publisher, &stopRequested, &tornReadDetected, &readCount]() {
        while (!stopRequested.load(std::memory_order_relaxed))
        {
            const PairValue& snapshot = publisher.AcquireReadSnapshot();
            if (snapshot.b != snapshot.a * 2)
            {
                tornReadDetected.store(true, std::memory_order_relaxed);
            }
            readCount.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopRequested.store(true, std::memory_order_relaxed);
    producer.join();
    consumer.join();

    EXPECT_FALSE(tornReadDetected.load());
    EXPECT_GT(readCount.load(), 0u);
}
