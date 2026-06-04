#include "EnemyRLPolicy.hpp"
#include "Grid.hpp"

#include <gtest/gtest.h>

TEST(EnemyRLPolicyTest, StateEncodingStable) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    const int s1 = EnemyRLPolicy::encodeState({5, 5}, {3, 5}, false, false, grid);
    const int s2 = EnemyRLPolicy::encodeState({5, 5}, {3, 5}, false, false, grid);
    EXPECT_EQ(s1, s2);
    EXPECT_GE(s1, 0);
    EXPECT_LT(s1, EnemyRLPolicy::kStateCount);
}

TEST(EnemyRLPolicyTest, LoadsBundledQTableIfPresent) {
    EnemyRLPolicy policy;
    const bool ok = policy.tryLoad("assets/ai/enemy_qtable.json") ||
                    policy.tryLoad("../assets/ai/enemy_qtable.json");
    if (ok) {
        EXPECT_TRUE(policy.isLoaded());
        EXPECT_GE(policy.bestAction(0), 0);
        EXPECT_LT(policy.bestAction(0), EnemyRLPolicy::kActionCount);
    }
}
