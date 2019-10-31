// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "WatchDog.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"

#include "ib/mw/sync/string_utils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;


class WatchDogTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(WarnHandler, void(std::chrono::milliseconds));
        MOCK_METHOD1(ErrorHandler, void(std::chrono::milliseconds));
    };

protected:
    WatchDogTest()
    {
    }

protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    Callbacks callbacks;
};

TEST_F(WatchDogTest, warn_after_timeout)
{
    WatchDog watchDog{10ms, std::chrono::milliseconds::max()};
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, error_after_timeout)
{
    WatchDog watchDog{10ms, 50ms};
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, warn_only_once)
{
    WatchDog watchDog{10ms, std::chrono::milliseconds::max()};
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, error_only_once)
{
    WatchDog watchDog{10ms, 50ms};
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, no_callback_if_reset_in_time)
{
    WatchDog watchDog{2s, 3s};
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();
    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(1s);
    watchDog.Reset();
}



} // anonymous namespace for test
