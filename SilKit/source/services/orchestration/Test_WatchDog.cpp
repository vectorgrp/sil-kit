/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


#include <chrono>
#include <thread>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/orchestration/string_utils.hpp"
#include "ParticipantConfiguration.hpp"
#include "WatchDog.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Services::Orchestration;


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

TEST_F(WatchDogTest, throw_if_warn_timeout_is_zero)
{
    EXPECT_THROW(WatchDog(Config::HealthCheck{ 0ms,10ms }), SilKitError);
}

TEST_F(WatchDogTest, throw_if_error_timeout_is_zero)
{
    EXPECT_THROW(WatchDog(Config::HealthCheck{ 10ms,0ms }), SilKitError);
}

TEST_F(WatchDogTest, warn_after_timeout)
{
    
    WatchDog watchDog{ Config::HealthCheck{10ms, std::chrono::milliseconds::max()} };
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, error_after_timeout)
{
    WatchDog watchDog{ Config::HealthCheck{10ms, 50ms} };
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, warn_only_once)
{

    WatchDog watchDog{ Config::HealthCheck{10ms, std::chrono::milliseconds::max()} };
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, error_only_once)
{
    WatchDog watchDog{ Config::HealthCheck{10ms, 50ms} };
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, no_callback_if_reset_in_time)
{
    WatchDog watchDog{ Config::HealthCheck{2000ms, 3000ms} };
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();
    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(1s);
    watchDog.Reset();
}

TEST_F(WatchDogTest, create_health_check_unconfigured)
{
    Config::HealthCheck healthCheck;
    WatchDog watchDog(healthCheck);
    EXPECT_EQ(watchDog.GetWarnTimeout(), watchDog._defaultTimeout);
    EXPECT_EQ(watchDog.GetErrorTimeout(), watchDog._defaultTimeout);

    healthCheck.softResponseTimeout = 5ms;
    WatchDog watchDogWithSoftTimeout{healthCheck};
    EXPECT_EQ(watchDogWithSoftTimeout.GetWarnTimeout(), 5ms);
    healthCheck.softResponseTimeout.reset();

    healthCheck.hardResponseTimeout = 5ms;
    WatchDog watchDogWithHardTimeout{healthCheck};
    EXPECT_EQ(watchDogWithHardTimeout.GetErrorTimeout(), 5ms);

}
TEST_F(WatchDogTest, create_health_check_configured)
{
    WatchDog watchDog{Config::HealthCheck{2000ms, 3000ms}};
    watchDog.SetWarnHandler([this](auto timeout) {
        callbacks.WarnHandler(timeout);
    });
    watchDog.SetErrorHandler([this](auto timeout) {
        callbacks.ErrorHandler(timeout);
    });

    watchDog.Start();
    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(1s);
    watchDog.Reset();
}

} // anonymous namespace
