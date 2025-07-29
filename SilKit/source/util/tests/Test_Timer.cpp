// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "Timer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace std::chrono_literals;

TEST(Test_Timer, ensure_util_timer_works)
{
    {
        //Make sure DTor is able to stop a running timer
        SilKit::Util::Timer timer;
        timer.WithPeriod(std::chrono::milliseconds(50), [](const auto) {});
    }

    {
        std::promise<void> done;
        auto isDone = done.get_future();
        SilKit::Util::Timer timer;
        auto numCalls = 0u;
        auto cb = [&](const auto) {
            numCalls++;
            if (numCalls == 5)
            {
                timer.Stop();
                done.set_value();
            }
        };
        timer.WithPeriod(std::chrono::milliseconds(50), cb);
        ASSERT_TRUE(timer.IsActive());
        isDone.wait_for(1s);
        ASSERT_TRUE(!timer.IsActive());
        ASSERT_EQ((int)numCalls, 5);
    }
}

} // namespace
