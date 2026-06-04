#include "SaveData.hpp"

#include <gtest/gtest.h>

TEST(SaveDataTest, RoundTrip) {
    const std::string path = "save/test_profile.save";
    SaveData data;
    data.highScore = 420;
    data.uiFontPercent = 125;
    data.lastDifficulty = 2;
    data.lastPlayMode = PlayMode::LOCAL_DUO;
    ASSERT_TRUE(data.save(path));

    SaveData loaded;
    ASSERT_TRUE(loaded.load(path));
    EXPECT_EQ(loaded.highScore, 420);
    EXPECT_EQ(loaded.uiFontPercent, 125);
    EXPECT_EQ(loaded.lastDifficulty, 2);
    EXPECT_EQ(loaded.lastPlayMode, PlayMode::LOCAL_DUO);
}
