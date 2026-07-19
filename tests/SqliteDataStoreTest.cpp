#include <gtest/gtest.h>

#include <stdexcept>

#include "serialization/SqliteDataStore.h"

TEST(SqliteDataStoreTest, SaveThrowsNotImplemented)
{
    SqliteDataStore store;
    const DataRecord record;
    EXPECT_THROW(store.Save("key", record), std::logic_error);
}

TEST(SqliteDataStoreTest, LoadThrowsNotImplemented)
{
    SqliteDataStore store;
    DataRecord record;
    EXPECT_THROW(store.Load("key", record), std::logic_error);
}
