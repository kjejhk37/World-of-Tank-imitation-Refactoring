#include <gtest/gtest.h>

#include "persistence/PlayerProgress.h"
#include "persistence/SaveLoadManager.h"
#include "serialization/IDataStore.h"

namespace
{
    class FakeDataStore : public IDataStore
    {
    public:
        DataRecord saved;
        bool loadSucceeds = true;

        bool Save(const std::string&, const DataRecord& record) override
        {
            saved = record;
            return true;
        }

        bool Load(const std::string&, DataRecord& outRecord) override
        {
            if (!loadSucceeds)
            {
                return false;
            }
            outRecord = saved;
            return true;
        }
    };
}

TEST(SaveLoadManagerTest, SaveIncludesSaveVersionField)
{
    FakeDataStore store;
    const SaveLoadManager manager(store);

    PlayerProgress progress;
    progress.currency = 100;
    progress.unlockedTankIds = {"tank_a", "tank_b"};

    ASSERT_TRUE(manager.Save("save.json", progress));
    EXPECT_EQ(store.saved.at("saveVersion"), "1");
    EXPECT_EQ(store.saved.at("currency"), "100");
}

TEST(SaveLoadManagerTest, SaveThenLoadRoundTripsData)
{
    FakeDataStore store;
    const SaveLoadManager manager(store);

    PlayerProgress saved;
    saved.currency = 250;
    saved.unlockedTankIds = {"tiger", "sherman"};
    ASSERT_TRUE(manager.Save("save.json", saved));

    PlayerProgress loaded;
    ASSERT_TRUE(manager.Load("save.json", loaded));
    EXPECT_EQ(loaded.currency, 250);
    EXPECT_EQ(loaded.unlockedTankIds, saved.unlockedTankIds);
}

TEST(SaveLoadManagerTest, LoadReturnsFalseWhenStoreLoadFails)
{
    FakeDataStore store;
    store.loadSucceeds = false;
    const SaveLoadManager manager(store);

    PlayerProgress progress;
    EXPECT_FALSE(manager.Load("missing.json", progress));
}
