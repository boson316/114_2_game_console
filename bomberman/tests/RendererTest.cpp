#include "Renderer.hpp"

#include <gtest/gtest.h>

TEST(RendererTest, DistinctWallChars) {
    EXPECT_NE(Renderer::CHAR_INDESTRUCTIBLE, Renderer::CHAR_DESTRUCTIBLE);
    EXPECT_NE(Renderer::CHAR_PLAYER, Renderer::CHAR_ENEMY);
}
