#include <gtest/gtest.h>

#include "gameboy/core/cycle_count.hpp"

TEST(CycleCountTest, StoresMachineCyclesExplicitly) {
    constexpr gameboy::CycleCount elapsed{3};

    EXPECT_EQ(elapsed.m_cycles(), 3U);
}
