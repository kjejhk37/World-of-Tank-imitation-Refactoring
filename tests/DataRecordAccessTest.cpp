#include <gtest/gtest.h>

#include "platform/serialization/DataRecordAccess.h"

TEST(DataRecordAccessTest, GetIntOrReturnsFallbackWhenFieldMissing)
{
    const DataRecord record;
    EXPECT_EQ(DataRecordAccess::GetIntOr(record, "width", 1280), 1280);
}

TEST(DataRecordAccessTest, GetIntOrParsesValidValue)
{
    const DataRecord record{{"width", "1920"}};
    EXPECT_EQ(DataRecordAccess::GetIntOr(record, "width", 1280), 1920);
}

TEST(DataRecordAccessTest, GetIntOrReturnsFallbackOnMalformedValue)
{
    const DataRecord record{{"width", "abc"}};
    EXPECT_EQ(DataRecordAccess::GetIntOr(record, "width", 1280), 1280);
}

TEST(DataRecordAccessTest, GetBoolOrParsesTrueAndFalse)
{
    const DataRecord record{{"fullscreen", "true"}};
    EXPECT_TRUE(DataRecordAccess::GetBoolOr(record, "fullscreen", false));

    const DataRecord record2{{"fullscreen", "false"}};
    EXPECT_FALSE(DataRecordAccess::GetBoolOr(record2, "fullscreen", true));
}

TEST(DataRecordAccessTest, GetBoolOrReturnsFallbackOnMalformedValue)
{
    const DataRecord record{{"fullscreen", "yes"}};
    EXPECT_TRUE(DataRecordAccess::GetBoolOr(record, "fullscreen", true));
}

TEST(DataRecordAccessTest, GetFloatOrParsesValidValue)
{
    const DataRecord record{{"volume", "0.5"}};
    EXPECT_FLOAT_EQ(DataRecordAccess::GetFloatOr(record, "volume", 1.0f), 0.5f);
}

TEST(DataRecordAccessTest, GetFloatOrReturnsFallbackOnMalformedValue)
{
    const DataRecord record{{"volume", "loud"}};
    EXPECT_FLOAT_EQ(DataRecordAccess::GetFloatOr(record, "volume", 1.0f), 1.0f);
}

TEST(DataRecordAccessTest, GetStringOrReturnsFallbackWhenFieldMissing)
{
    const DataRecord record;
    EXPECT_EQ(DataRecordAccess::GetStringOr(record, "renderer", "directx"), "directx");
}

TEST(DataRecordAccessTest, GetStringOrReturnsStoredValue)
{
    const DataRecord record{{"renderer", "opengl"}};
    EXPECT_EQ(DataRecordAccess::GetStringOr(record, "renderer", "directx"), "opengl");
}
