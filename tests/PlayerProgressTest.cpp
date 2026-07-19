#include <gtest/gtest.h>

#include "persistence/PlayerProgress.h"

TEST(PlayerProgressTest, FromRecordKeepsExistingCurrencyOnMalformedValue)
{
    PlayerProgress progress;
    progress.currency = 42;

    const DataRecord record{{"currency", "not_a_number"}};
    progress.FromRecord(record);

    EXPECT_EQ(progress.currency, 42);
}

TEST(PlayerProgressTest, FromRecordParsesValidCurrency)
{
    PlayerProgress progress;

    const DataRecord record{{"currency", "500"}};
    progress.FromRecord(record);

    EXPECT_EQ(progress.currency, 500);
}
