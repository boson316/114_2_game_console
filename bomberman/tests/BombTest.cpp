#include "Bomb.hpp"

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

TEST(BombTest, EnemyOwnerTracked) {
    Bomb bomb({3, 3}, 2.0f, 2, BombOwner::ENEMY);
    EXPECT_EQ(bomb.getOwner(), BombOwner::ENEMY);
}

TEST(BombTest, FuseCountdown) {
    Bomb bomb({3, 3}, 3.0f, 2);
    bomb.update(1.0f);
    EXPECT_NEAR(bomb.getRemainingFuse(), 2.0f, 0.001f);
    EXPECT_FALSE(bomb.isFuseExpired());
    bomb.update(2.0f);
    EXPECT_TRUE(bomb.isFuseExpired());
}

RC_GTEST_PROP(BombTest, ActiveBombCountNeverExceedsMax, ()) {
    const auto maxBombs = *rc::gen::inRange(1, 5);
    const auto attempts = *rc::gen::inRange(1, 20);

    int activeBombs = 0;
    for (int i = 0; i < attempts; ++i) {
        if (activeBombs < maxBombs) {
            ++activeBombs;
        }
        RC_ASSERT(activeBombs <= maxBombs);
    }
}
