#include <gtest/gtest.h>

#include "projects/config/ConfigManager.h"

namespace
{
    class FakeDataStore : public IDataStore
    {
    public:
        DataRecord recordToReturn;
        bool loadSucceeds = true;

        bool Save(const std::string&, const DataRecord&) override
        {
            return true;
        }

        bool Load(const std::string&, DataRecord& outRecord) override
        {
            if (!loadSucceeds)
            {
                return false;
            }
            outRecord = recordToReturn;
            return true;
        }
    };
}

TEST(ConfigManagerTest, ReturnsHardcodedDefaultsWhenFileMissing)
{
    FakeDataStore store;
    store.loadSucceeds = false;

    const ConfigManager manager(store);
    const AppConfig config = manager.LoadOrDefault("missing.json");

    EXPECT_EQ(config.width, 1280);
    EXPECT_EQ(config.height, 720);
    EXPECT_FALSE(config.fullscreen);
    EXPECT_FLOAT_EQ(config.volume, 1.0f);
    EXPECT_EQ(config.renderer, RendererBackend::DirectX11);
    EXPECT_EQ(config.windowTitle, AppConfig::kDefaultWindowTitle);
    EXPECT_TRUE(config.logFilePath.empty());
}

TEST(ConfigManagerTest, OverridesDefaultsWithFileValues)
{
    FakeDataStore store;
    store.recordToReturn = {{"width", "1920"},
                             {"height", "1080"},
                             {"fullscreen", "true"},
                             {"volume", "0.5"},
                             {"renderer", "opengl"},
                             {"windowTitle", "Custom Title"},
                             {"logFilePath", "logs/app.txt"}};

    const ConfigManager manager(store);
    const AppConfig config = manager.LoadOrDefault("config.json");

    EXPECT_EQ(config.width, 1920);
    EXPECT_EQ(config.height, 1080);
    EXPECT_TRUE(config.fullscreen);
    EXPECT_FLOAT_EQ(config.volume, 0.5f);
    EXPECT_EQ(config.renderer, RendererBackend::OpenGL);
    EXPECT_EQ(config.windowTitle, "Custom Title");
    EXPECT_EQ(config.logFilePath, "logs/app.txt");
}

TEST(ConfigManagerTest, FallsBackToDefaultOnUnknownRendererValue)
{
    FakeDataStore store;
    store.recordToReturn = {{"renderer", "not_a_backend"}};

    const ConfigManager manager(store);
    const AppConfig config = manager.LoadOrDefault("config.json");

    EXPECT_EQ(config.renderer, RendererBackend::DirectX11);
}

TEST(ConfigManagerTest, FallsBackToDefaultOnNonNumericWidth)
{
    FakeDataStore store;
    store.recordToReturn = {{"width", "abc"}};

    const ConfigManager manager(store);
    const AppConfig config = manager.LoadOrDefault("config.json");

    EXPECT_EQ(config.width, 1280);
}
