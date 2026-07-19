#include <gtest/gtest.h>

#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "logging/Logger.h"

namespace
{
    class FakeLogSink : public ILogSink
    {
    public:
        explicit FakeLogSink(std::vector<std::pair<LogLevel, std::string>>& calls)
            : m_calls(calls)
        {
        }

        void Write(LogLevel level, const std::string& formattedLine) override
        {
            m_calls.emplace_back(level, formattedLine);
        }

    private:
        std::vector<std::pair<LogLevel, std::string>>& m_calls;
    };
}

TEST(LoggerTest, InfoDispatchesInfoLevelWithMessage)
{
    std::vector<std::pair<LogLevel, std::string>> calls;
    std::vector<std::unique_ptr<ILogSink>> sinks;
    sinks.push_back(std::make_unique<FakeLogSink>(calls));
    Logger logger(std::move(sinks));

    logger.Info("hello");

    ASSERT_EQ(calls.size(), 1u);
    EXPECT_EQ(calls[0].first, LogLevel::Info);
    EXPECT_NE(calls[0].second.find("Info : "), std::string::npos);
    EXPECT_NE(calls[0].second.find("hello"), std::string::npos);
}

TEST(LoggerTest, WarningDispatchesWarningLevelWithMessage)
{
    std::vector<std::pair<LogLevel, std::string>> calls;
    std::vector<std::unique_ptr<ILogSink>> sinks;
    sinks.push_back(std::make_unique<FakeLogSink>(calls));
    Logger logger(std::move(sinks));

    logger.Warning("careful");

    ASSERT_EQ(calls.size(), 1u);
    EXPECT_EQ(calls[0].first, LogLevel::Warning);
    EXPECT_NE(calls[0].second.find("careful"), std::string::npos);
}

TEST(LoggerTest, ErrorEmbedsErrorCodeInMessage)
{
    std::vector<std::pair<LogLevel, std::string>> calls;
    std::vector<std::unique_ptr<ILogSink>> sinks;
    sinks.push_back(std::make_unique<FakeLogSink>(calls));
    Logger logger(std::move(sinks));

    logger.Error(ErrorCode::WindowCreationFailed, "failed to create window");

    ASSERT_EQ(calls.size(), 1u);
    EXPECT_EQ(calls[0].first, LogLevel::Error);
    EXPECT_NE(calls[0].second.find("[2001]"), std::string::npos);
    EXPECT_NE(calls[0].second.find("failed to create window"), std::string::npos);
}

TEST(LoggerTest, DispatchesToAllRegisteredSinks)
{
    std::vector<std::pair<LogLevel, std::string>> firstCalls;
    std::vector<std::pair<LogLevel, std::string>> secondCalls;
    std::vector<std::unique_ptr<ILogSink>> sinks;
    sinks.push_back(std::make_unique<FakeLogSink>(firstCalls));
    sinks.push_back(std::make_unique<FakeLogSink>(secondCalls));
    Logger logger(std::move(sinks));

    logger.Info("broadcast");

    EXPECT_EQ(firstCalls.size(), 1u);
    EXPECT_EQ(secondCalls.size(), 1u);
}

TEST(LoggerTest, ConcurrentCallsFromMultipleThreadsDoNotLoseOrCorruptEntries)
{
    constexpr int kThreadCount = 8;
    constexpr int kMessagesPerThread = 50;

    std::vector<std::pair<LogLevel, std::string>> calls;
    std::vector<std::unique_ptr<ILogSink>> sinks;
    sinks.push_back(std::make_unique<FakeLogSink>(calls));
    Logger logger(std::move(sinks));

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreadCount; ++i)
    {
        threads.emplace_back([&logger, kMessagesPerThread]() {
            for (int j = 0; j < kMessagesPerThread; ++j)
            {
                logger.Info("concurrent");
            }
        });
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    // std::mutex로 보호되는 분배 구간이 없다면 std::vector에 대한 동시 emplace_back으로 크래시하거나
    // 호출 수가 유실될 수 있다 — 정확히 (스레드 수 × 호출 수)만큼 기록됐는지로 mutex 보호를 검증한다.
    EXPECT_EQ(calls.size(), static_cast<std::size_t>(kThreadCount * kMessagesPerThread));
}
