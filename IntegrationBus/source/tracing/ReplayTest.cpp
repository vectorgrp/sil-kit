// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockComAdapter.hpp"
#include "Timer.hpp"

namespace {

using namespace ib::mw::test;

TEST(ReplayTest, ensure_util_timer_works)
{

    {
        ib::util::Timer timer;
        timer.WithPeriod(std::chrono::milliseconds(50), [](const auto) {});
    }

    {
        ib::util::Timer timer;
        auto numCalls = 0u;
        auto cb = [&](const auto now) {
            numCalls++;
            if (numCalls == 5)
            {
                timer.Stop();
            }
        };
        timer.WithPeriod(std::chrono::milliseconds(50), cb);
        ASSERT_TRUE(timer.IsActive());
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * (5 + 1)));
        ASSERT_TRUE(!timer.IsActive());
        ASSERT_EQ(numCalls, 5);
    }
}

TEST(ReplayTest, replaytask_is_ordered)
{

}

}
