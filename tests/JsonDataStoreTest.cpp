#include <gtest/gtest.h>

#include <cstdio>

#include "serialization/JsonDataStore.h"

namespace
{
    constexpr const char* kTestFilePath = "json_data_store_test_tmp.json";

    class JsonDataStoreTest : public ::testing::Test
    {
    protected:
        void TearDown() override
        {
            std::remove(kTestFilePath);
        }
    };
}

TEST_F(JsonDataStoreTest, SaveThenLoadRoundTripsValues)
{
    JsonDataStore store;
    const DataRecord record{{"width", "1920"}, {"fullscreen", "true"}};

    ASSERT_TRUE(store.Save(kTestFilePath, record));

    DataRecord loaded;
    ASSERT_TRUE(store.Load(kTestFilePath, loaded));
    EXPECT_EQ(loaded.at("width"), "1920");
    EXPECT_EQ(loaded.at("fullscreen"), "true");
}

TEST_F(JsonDataStoreTest, LoadReturnsFalseWhenFileMissing)
{
    JsonDataStore store;
    DataRecord loaded;
    EXPECT_FALSE(store.Load("json_data_store_test_nonexistent.json", loaded));
}
